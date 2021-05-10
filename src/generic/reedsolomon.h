/*
 * reedsolomon.h
 *
 *  Created on: 04/11/2016
 *      Author: Lucas Teske
 */

#ifndef GENERIC_REEDSOLOMON_H
#define GENERIC_REEDSOLOMON_H

#include <cstdint>

extern "C" {
#include <correct.h>
}

namespace SatHelper {
    class ReedSolomon {
        public:
            ReedSolomon();
            ~ReedSolomon();

            // CCSDS standard (255,223) RS codec with dual-basis symbol representation
            ssize_t decode_ccsds(uint8_t *data);
            // CCSDS standard (255,223) RS codec with conventional (*not* dual-basis) symbol representation
            ssize_t decode_rs8(uint8_t *data);

            // Deinterleave by I
            void deinterleave(const uint8_t *data, uint8_t *output, uint8_t pos, uint8_t I);
            // Interleave by I
            void interleave(const uint8_t *data, uint8_t *output, uint8_t pos, uint8_t I);

        private:
            correct_reed_solomon *rs;
    };
}

#endif
