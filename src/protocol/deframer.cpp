/*
 * LeanHRPT Decode
 * Copyright (C) 2021 Xerbo
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "deframer.h"

#include <cstring>
#include <stdexcept>
#include <bitset>

// Constructor
template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::ArbitraryDeframer(unsigned int incorrectBitThreshold, bool checkInverted)
    : frameBuffer(new uint8_t[FRAME_SIZE / 8]),
      checkInverted(checkInverted),
      incorrectBitThreshold(incorrectBitThreshold) {
    if (ASM_SIZE > sizeof(ASM_T)*8) {
        throw std::runtime_error("ArbitraryDeframer: ASM size larger than what ASM_T allows");
    }
}

template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::~ArbitraryDeframer() {
    delete[] frameBuffer;
}

// Push a byte into the buffer
template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
void ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::pushByte(uint8_t byte) {
    frameBuffer[bufferPosition++] = byte;
}

// Push a bit into the buffer
template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
void ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::pushBit(bool bit) {
    byteBuffer = byteBuffer << 1 | bit;
    bufferBitPosition++;

    if(bufferBitPosition == 8){
        pushByte(byteBuffer);
        bufferBitPosition = 0;
    }
}

template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
bool ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::fuzzyBitCompare(ASM_T a, ASM_T b, size_t threshold) {
    return std::bitset<ASM_SIZE>(a ^ b).count() <= threshold;
}

// Push a bit into the buffer
template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
void ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::startWriting() {
    bufferPosition = 0;
    bufferBitPosition = 0;
    bitsWritten = 0;
    writingData = true;
}

// Work function
template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
bool ArbitraryDeframer<ASM_T, ASM, ASM_SIZE, FRAME_SIZE>::work(const uint8_t *data, uint8_t *out, unsigned int len) {
    bool complete_frame = false;

    for(size_t i = 0; i < len; i++) {
        for(int j = 7; j >= 0; j--) {
            bool bit = std::bitset<ASM_SIZE>(data[i]).test(j);
            if(invert) bit = !bit;

            shifter = shifter << 1 | bit;

            if(writingData) {
                // Append syncword, backwards
                if(bitsWritten == 0) {
                    for(int ASMBit = ASM_SIZE-1; ASMBit >= 0; ASMBit--) {
                        pushBit(std::bitset<ASM_SIZE>(ASM).test(ASMBit));
                    }
                    bitsWritten += ASM_SIZE;
                }

                pushBit(bit);
                bitsWritten++;

                // At the end of a frame, copy the data into the output pointer and reset
                if(bitsWritten == FRAME_SIZE) {
                    writingData = false;
                    bitsWritten = 0;

                    complete_frame = 1;
                    std::memcpy(out, frameBuffer, FRAME_SIZE / 8);

                    continue;
                }
            }

            if(fuzzyBitCompare(shifter, ASM, incorrectBitThreshold)) {
                startWriting();
            } else if(checkInverted && fuzzyBitCompare(shifter, ~ASM, incorrectBitThreshold)) {
                startWriting();
                invert = !invert;
            }
        }
    }

    return complete_frame;
}

// Standard 8192 bit CCSDS frames
// DEPRECATED: Use ccsds::Deframer instead
//template class ArbitraryDeframer<uint32_t, 0x1ACFFC1D, 32, 8192>;

// Meteor MSU-MR
template class ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 11850 * 8>;
// Meteor Telemetry
template class ArbitraryDeframer<uint64_t, 0x0218A7A392DD9ABF, 64, 74 * 8>;
// Meteor MTVZA
template class ArbitraryDeframer<uint32_t, 0xFB386A45, 32, 248 * 8>;

// NOAA HRPT
template class ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 110900>;
// NOAA GAC
template class ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 33270>;

// Fengyun VIRR
template class ArbitraryDeframer<uint64_t, 0b101000010001011011111101011100011001110110000011110010010101, 60, 208400>;
