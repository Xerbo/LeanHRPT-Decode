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

#ifndef LEANHRPT_CCSDS_DEFRAMER_H
#define LEANHRPT_CCSDS_DEFRAMER_H

#include <cstddef>
#include <cstdint>

namespace ccsds {
enum SyncMachineState { State0, State1, State2, State3 };

using asm_t = uint32_t;

/// A deframer based on http://www.sat.cc.ua/data/CADU%20Frame%20Synchro.pdf
class Deframer {
   public:
    Deframer();
    ~Deframer();
    bool work(const uint8_t *in, uint8_t *out, size_t len);

   private:
    asm_t shifter;
    bool asmCompare(asm_t a, asm_t b);

    uint8_t *frameBuffer;
    uint8_t byteBuffer;
    unsigned int bufferPosition = 0, bufferBitPosition = 0;
    void pushByte(uint8_t byte);
    void pushBit(bool bit);

    SyncMachineState state;

    unsigned int bitsWritten;
    bool writingData;
    bool invert;
    unsigned int badFrames;
    unsigned int goodFrames;

    void startWriting();
    void enterState(SyncMachineState newState);
    int skip;
};
}  // namespace ccsds

#endif
