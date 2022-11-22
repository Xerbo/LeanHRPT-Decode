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

#ifndef LEANHRPT_PROTOCOL_DEFRAMER_H_
#define LEANHRPT_PROTOCOL_DEFRAMER_H_

#include <cstddef>
#include <cstdint>

template <typename ASM_T, ASM_T ASM, unsigned int ASM_SIZE, unsigned int FRAME_SIZE>
class ArbitraryDeframer {
   public:
    ArbitraryDeframer(unsigned int incorrectBitThreshold = 10, bool checkInverted = false);
    ~ArbitraryDeframer();
    bool work(const uint8_t *data, uint8_t *out, unsigned int len);

   private:
    uint8_t *frameBuffer;

    // Options
    bool checkInverted;
    unsigned int incorrectBitThreshold;

    // Used for loading data into `frameBuffer`
    uint8_t byteBuffer;
    int bufferPosition = 0, bufferBitPosition = 0;
    void pushByte(uint8_t byte);
    void pushBit(bool bit);

    // Actually used for deframing
    ASM_T shifter;
    unsigned int bitsWritten = 0;
    bool writingData = false;
    bool invert = false;
    void startWriting();
    bool fuzzyBitCompare(ASM_T a, ASM_T b, size_t threshold);
};

#endif
