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

#include "FcsCommunication.h"
#include "Logger.h"
#include "TcpServer.h"
#include "VerifierProtocol.h"

#include <map>
#include <signal.h>
#include <string>

TcpServer server;

void onSignal(sig_atomic_t s)
{
    if (s == SIGINT || s == SIGTERM)
    {
        Logger::log("Signal caught, terminating");
        server.closeSockets();
        exit(0);
    }
}

void handleIncomingMessage(unsigned char *messageBuffer, size_t messageSize,
    unsigned char **responseBuffer, size_t &responseSize)
{
    VerifierProtocol verifierProtocol;
    if (!verifierProtocol.parseMessage(messageBuffer, messageSize))
    {
        Logger::log("Couldn't parse incoming message", Error);
        verifierProtocol.prepareResponseMessage(
            nullptr, 0, responseBuffer, responseSize,
            verifierProtocol.getErrorCode());
        return;
    }

    size_t outPayloadSize = 0;
    unsigned char *outPayloadBuffer = nullptr;
    int fcsStatus;
    switch (verifierProtocol.getCommandCode())
    {
        case getChipId:
        {
            fcsStatus = FcsCommunication::getChipId(
                outPayloadSize, &outPayloadBuffer);
        }
        break;
        case sigmaTeardown:
        {
            fcsStatus = FcsCommunication::sigmaTeardown(
                verifierProtocol.getSigmaTeardownSessionId());
        }
        break;
        case createAttestationSubKey:
        {
            fcsStatus = FcsCommunication::createAttestationSubkey(
                verifierProtocol.getIncomingPayload(),
                verifierProtocol.getIncomingPayloadSize(),
                &outPayloadBuffer,
                outPayloadSize);
        }
        break;
        case getMeasurement:
        {
            fcsStatus = FcsCommunication::getMeasurement(
                verifierProtocol.getIncomingPayload(),
                verifierProtocol.getIncomingPayloadSize(),
                &outPayloadBuffer,
                outPayloadSize);
        }
        break;
        default:
        {
            Logger::log("Command code not recognized: "
                + std::to_string(verifierProtocol.getCommandCode()));
            verifierProtocol.prepareResponseMessage(
                nullptr, 0, responseBuffer, responseSize, unknownCommand);
            return;
        }
        break;
    }
    if (fcsStatus == FcsCommunication::kIoctlFailedError)
    {
        /*
        don't prepare any message, server should disconnect
        - emulates system console behavior
        */
        delete[] outPayloadBuffer;
        return;
    }
    verifierProtocol.prepareResponseMessage(
        outPayloadBuffer, outPayloadSize,
        responseBuffer, responseSize,
        fcsStatus);
    delete[] outPayloadBuffer;
}

void printUsageAndExit()
{
    Logger::log("Usage: <executable name> <port number> optional:<log level> e.g ./fcsServer 50001 Debug", Fatal);
    Logger::log("Possible log levels: Debug, Info (default), Error, Fatal", Fatal);
    exit(1);
}

int main(int argc, char* argv[])
{
    signal(SIGINT, onSignal);
    signal(SIGTERM, onSignal);
    if (argc == 3)
    {
        const std::map<std::string, LogLevel> LogLevelMap
        {
            { "Debug", Debug },
            { "Info", Info },
            { "Error", Error },
            { "Fatal", Fatal }
        };

        auto itr = LogLevelMap.find(argv[2]);
        if (itr == LogLevelMap.end())
        {
            printUsageAndExit();
        }
        Logger::setCurrentLogLevel(itr->second);
    }
    if (argc < 2 || argc > 3)
    {
        printUsageAndExit();
    }
    int portNumber;
    try
    {
        portNumber = std::stoi(argv[1]);
    }
    catch(const std::exception& e)
    {
        printUsageAndExit();
    }

    server.run(portNumber, &handleIncomingMessage);
    return 0;
}