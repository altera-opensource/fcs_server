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

#ifndef VERIFIERPROTOCOL_H
#define VERIFIERPROTOCOL_H

#include "CommandHeader.h"

#include <stddef.h>

#define RESERVED_BYTES_COUNT 4
#define WORD_SIZE 4
#define SIGMA_TEARDOWN_MAGIC 0xb852e2a4
#define SIGMA_TEARDOWN_COMMAND_SIZE 8
#define SIGMA_TEARDOWN_SESSIONID_OFFSET 4

enum CommandCode
{
    getChipId = 0x12,
    sigmaTeardown = 0xd5,
    createAttestationSubKey = 0x182,
    getMeasurement = 0x183
};

enum ErrorCode
{
    noError = 0x00,
    genericError = 0x01,
    unknownCommand = 0x03,
    invalidHeader = 0x04,
    invalidMagic = 0x80
};

class VerifierProtocol
{
    public:
        bool parseMessage(
            unsigned char *messageBuffer, size_t messageSize);
        void prepareResponseMessage(
            unsigned char *payloadBuffer, size_t payloadSize,
            unsigned char **responseBuffer, size_t &responseSize,
            const int returnCode);
        uint32_t getCommandCode();
        uint32_t getSigmaTeardownSessionId();

        unsigned char *getIncomingPayload()
        {
            return incomingPayload;
        }
        size_t getIncomingPayloadSize()
        {
            return incomingPayloadSize;
        }
        ErrorCode getErrorCode()
        {
            return errorCode;
        }

    private:
        bool isMagicWordCorrect();
        uint32_t getPayloadOffset();
        unsigned char *incomingPayload;
        size_t incomingPayloadSize;
        CommandHeader incomingHeader;
        ErrorCode errorCode = genericError;
};

#endif /* VERIFIERPROTOCOL_H */