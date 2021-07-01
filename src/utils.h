/*
This project, FPGA Crypto Service Server, is licensed as below

***************************************************************************

Copyright 2020-2021 Intel Corporation. All Rights Reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.

2. Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in the
documentation and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER
OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

***************************************************************************
*/

#ifndef UTILS_H
#define UTILS_H

#include <stdexcept>
#include <stdint.h>
#include <vector>

class Utils
{
    public:
        static uint32_t decodeFromLittleEndianBuffer(
            std::vector<uint8_t> &buffer, size_t offset = 0)
        {
            if (buffer.size() < offset + sizeof(uint32_t))
            {
                throw std::invalid_argument("decodeFromLittleEndianBuffer: buffer too small");
            }
            return (uint32_t)((((buffer)[3 + offset]) << 24)
                            + (((buffer)[2 + offset]) << 16)
                            + (((buffer)[1 + offset]) << 8)
                            + ((buffer)[0 + offset]));
        }

        static void encodeToLittleEndianBuffer(
            uint32_t uintValue, std::vector<uint8_t> &buffer, size_t offset = 0)
        {
            if (buffer.size() < offset + sizeof(uint32_t))
            {
                throw std::invalid_argument("encodeToLittleEndianBuffer: buffer too small");
            }
            buffer[3 + offset] = (unsigned char)(uintValue >> 24);
            buffer[2 + offset] = (unsigned char)(uintValue >> 16);
            buffer[1 + offset] = (unsigned char)(uintValue >> 8);
            buffer[0 + offset] = (unsigned char)uintValue;
        }
};

#endif /* UTILS_H */