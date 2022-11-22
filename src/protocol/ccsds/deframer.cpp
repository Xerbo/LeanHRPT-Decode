/*
 * LeanHRPT Decode
 * Copyright (C) 2021-2022 Xerbo
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

#include <bitset>
#include <cstring>
#include <stdexcept>

// Slightly modified from Oleg's original values
const unsigned int stateThresholds[4] = {0, 2, 6, 16};

#define ASM_SIZE 32
#define ASM 0x1ACFFC1D
#define INVERSE_ASM (~ASM)

#define FRAME_SIZE 8192
#define FRAME_SIZE_BYTES (FRAME_SIZE / 8)

namespace ccsds {
Deframer::Deframer()
    : shifter(0),
      frameBuffer(new uint8_t[FRAME_SIZE_BYTES]),
      byteBuffer(0),
      bufferPosition(0),
      bufferBitPosition(0),
      state(SyncMachineState::State0),
      bitsWritten(0),
      writingData(false),
      invert(false),
      badFrames(0),
      goodFrames(0),
      skip(0) {}
Deframer::~Deframer() { delete[] frameBuffer; }

bool Deframer::asmCompare(asm_t a, asm_t b) { return std::bitset<ASM_SIZE>(a ^ b).count() <= stateThresholds[(int)state]; }

// Push a byte into the buffer
void Deframer::pushByte(uint8_t byte) { frameBuffer[bufferPosition++] = byte; }

// Push a bit into the buffer
void Deframer::pushBit(bool bit) {
    byteBuffer = byteBuffer << 1 | bit;
    bufferBitPosition++;

    if (bufferBitPosition == 8) {
        pushByte(byteBuffer);
        bufferBitPosition = 0;
    }
}

void Deframer::startWriting() {
    bufferPosition = 0;
    bufferBitPosition = 0;
    bitsWritten = 0;
    writingData = true;
}

void Deframer::enterState(SyncMachineState newState) {
    goodFrames = 0;
    badFrames = 0;
    state = newState;
}

bool Deframer::work(const uint8_t *in, uint8_t *out, size_t len) {
    bool complete_frame = false;

    for (unsigned int i = 0; i < len; i++) {
        for (int j = 7; j >= 0; j--) {
            bool bit = std::bitset<ASM_SIZE>(in[i]).test(j);
            if (invert) bit = !bit;

            shifter = shifter << 1 | bit;

            if (writingData) {
                // Append syncword, backwards
                if (bitsWritten == 0) {
                    for (int ASMBit = ASM_SIZE - 1; ASMBit >= 0; ASMBit--) {
                        pushBit(std::bitset<ASM_SIZE>(ASM).test(ASMBit));
                    }
                    bitsWritten += ASM_SIZE;
                }

                pushBit(bit);
                bitsWritten++;

                // At the end of a frame, copy the data into the output pointer and reset
                if (bitsWritten == FRAME_SIZE) {
                    writingData = false;
                    bitsWritten = 0;
                    skip = ASM_SIZE;
                    complete_frame = true;
                    std::memcpy(out, frameBuffer, FRAME_SIZE_BYTES);
                }

                if (state != SyncMachineState::State1) continue;
            }

            // Skip until next sync marker
            if (skip > 1) {
                skip--;
                continue;
            }

            switch (state) {
                // Checks for a perfect sync marker with no errors, if we find one jump to State2
                case SyncMachineState::State0:
                    if (asmCompare(shifter, ASM)) {
                        enterState(SyncMachineState::State2);
                        startWriting();
                    } else if (asmCompare(shifter, INVERSE_ASM)) {
                        invert = !invert;
                        enterState(SyncMachineState::State2);
                        startWriting();
                    }
                    break;
                // Allow up to 2 bit errors, if we check 5 frames without success go back to State0
                // assuming we have lost all lock
                case SyncMachineState::State1:
                    if (asmCompare(shifter, ASM)) {
                        startWriting();
                        badFrames = 0;
                        enterState(SyncMachineState::State2);
                    } else {
                        badFrames++;
                        goodFrames = 0;
                        if (badFrames == 5) {
                            enterState(SyncMachineState::State0);
                        }
                    }
                    break;
                // Intermediate state between the strict State0 and lenient State3
                case SyncMachineState::State2:
                    if (asmCompare(shifter, ASM)) {
                        startWriting();
                        goodFrames++;
                        badFrames = 0;
                        if (goodFrames == 5) {
                            enterState(SyncMachineState::State3);
                        }
                    } else {
                        badFrames++;
                        goodFrames = 0;
                        if (badFrames == 2) {
                            enterState(SyncMachineState::State1);
                        }
                    }
                    break;
                // Assume fully locked, allow a very high level of errors
                case SyncMachineState::State3:
                    if (asmCompare(shifter, ASM)) {
                        startWriting();
                    } else {
                        enterState(SyncMachineState::State2);
                    }
                    break;
                default:
                    throw std::runtime_error("Invalid value in enum `SyncMachineState`");
            }
        }
    }

    return complete_frame;
}
}  // namespace ccsds
