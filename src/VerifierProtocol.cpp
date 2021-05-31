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

#include "Logger.h"
#include "utils.h"
#include "VerifierProtocol.h"

#include <string.h>

bool VerifierProtocol::parseMessage(
    unsigned char *messageBuffer, size_t messageSize)
{
    if (messageSize < CommandHeader::getRequiredSize())
    {
        Logger::log("Message Size is smaller than Command Header", Error);
        errorCode = invalidHeader;
        return false;
    }

    incomingHeader.parse(messageBuffer);

    size_t actualDataSizeInBytes = messageSize - CommandHeader::getRequiredSize();
    size_t dataSizeFromHeaderInBytes = incomingHeader.length * WORD_SIZE;
    if (actualDataSizeInBytes != dataSizeFromHeaderInBytes)
    {
        Logger::log("Data length received from network (bytes): "
            + std::to_string(actualDataSizeInBytes)
            + " is not equal to size specified in command header (converted from word to bytes): "
            + std::to_string(dataSizeFromHeaderInBytes), Error);
        errorCode = invalidHeader;
        return false;
    }

    uint32_t payloadOffset = getPayloadOffset();

    if (messageSize < payloadOffset)
    {
        Logger::log("Message Size too small", Error);
        errorCode = invalidHeader;
        return false;
    }

    incomingPayload = messageBuffer + payloadOffset;
    incomingPayloadSize = messageSize - payloadOffset;
    if (!isMagicWordCorrect())
    {
        return false;
    }
    errorCode = noError;
    return true;
}

uint32_t VerifierProtocol::getPayloadOffset()
{
    size_t messageReservedBytesCount = 0;
    if (incomingHeader.code == sigmaTeardown
        || incomingHeader.code == createAttestationSubKey
        || incomingHeader.code == getMeasurement)
    {
        messageReservedBytesCount = RESERVED_BYTES_COUNT;
    }

    return CommandHeader::getRequiredSize() + messageReservedBytesCount;
}

bool VerifierProtocol::isMagicWordCorrect()
{
    if (incomingHeader.code == sigmaTeardown)
    {
        if (incomingPayloadSize < SIGMA_TEARDOWN_COMMAND_SIZE)
        {
            Logger::log("Message Size too small", Error);
            errorCode = invalidHeader;
            return false;
        }
        uint32_t incomingMagic = Utils::decodeFromLittleEndianBuffer(
            incomingPayload);
        if (incomingMagic != SIGMA_TEARDOWN_MAGIC)
        {
            Logger::log("Incorrect sigma teardown magic: "
                + std::to_string(incomingMagic), Error);
            errorCode = invalidMagic;
            return false;
        }
    }
    return true;
}

void VerifierProtocol::prepareResponseMessage(
    unsigned char *payloadBuffer, size_t payloadSize,
    unsigned char **responseBuffer, size_t &responseSize,
    const int returnCode)
{
    Logger::log("Preparing response with return code "
        + std::to_string(returnCode));
    if (payloadSize % WORD_SIZE != 0)
    {
        Logger::log("Payload size not divisible by word size", Error);
        prepareResponseMessage(
            nullptr, 0, responseBuffer, responseSize, genericError);
        return;
    }

    size_t responsePayloadOffset = CommandHeader::getRequiredSize();
    responseSize = payloadSize + responsePayloadOffset;
    size_t lengthInWords = payloadSize / WORD_SIZE;

    *responseBuffer = new unsigned char[responseSize];

    CommandHeader outgoingHeader;
    outgoingHeader.client = incomingHeader.client;
    outgoingHeader.id = incomingHeader.id;
    outgoingHeader.length = lengthInWords;
    outgoingHeader.res1 = 0;
    outgoingHeader.res2 = 0;
    outgoingHeader.code = returnCode;

    if (payloadSize == 0 || payloadBuffer == nullptr)
    {
        outgoingHeader.length = 0;
    }
    else
    {
        Logger::log("copying payload to response buffer", Debug);
        memcpy(*responseBuffer + responsePayloadOffset,
            payloadBuffer,
            payloadSize);
    }
    Logger::log("encoding header", Debug);
    outgoingHeader.encode(*responseBuffer);
}

uint32_t VerifierProtocol::getCommandCode()
{
    return incomingHeader.code;
}

uint32_t VerifierProtocol::getSigmaTeardownSessionId()
{
    if (getCommandCode() != sigmaTeardown)
    {
        Logger::log("Attempt to read SessionId from message of type other than sigmaTeardown", Fatal);
        exit(1);
    }
    return Utils::decodeFromLittleEndianBuffer(
        incomingPayload + SIGMA_TEARDOWN_SESSIONID_OFFSET);
}