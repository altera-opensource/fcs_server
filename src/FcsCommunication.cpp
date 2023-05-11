/*
This project, FPGA Crypto Service Server, is licensed as below

***************************************************************************

Copyright 2020-2023 Intel Corporation. All Rights Reserved.

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
#include "utils.h"

#include "intel_fcs-ioctl.h"
#include "intel_fcs_structs.h"

#include <fcntl.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

#define FCS_DEVICE_PATH "/dev/fcs"

bool FcsCommunication::sendIoctl(
    intel_fcs_dev_ioctl *data,
    unsigned long commandCode)
{
    data->status = -1;
    int deviceFileDescriptor = open(FCS_DEVICE_PATH, O_RDWR);
    if (deviceFileDescriptor < 0)
    {
        Logger::logWithReturnCode("Opening device failed.", errno, Error);
        return false;
    }
    if (ioctl(deviceFileDescriptor, commandCode, data) < 0)
    {
        close(deviceFileDescriptor);
        Logger::logWithReturnCode("Ioctl failed.", errno, Error);
        return false;
    }
    close(deviceFileDescriptor);
    Logger::logWithReturnCode("Ioctl success.", data->status, Debug);
    return true;
}

bool FcsCommunication::getChipId(
    std::vector<uint8_t> &outBuffer, int32_t &fcsStatus)
{
    Logger::log("Calling getChipId");
    intel_fcs_dev_ioctl data = {};
    if (!sendIoctl(&data, INTEL_FCS_DEV_CHIP_ID))
    {
        return false;
    }
    outBuffer.resize(sizeof(data.com_paras.c_id));

    Utils::encodeToLittleEndianBuffer(
        data.com_paras.c_id.chip_id_low,
        outBuffer);
    Utils::encodeToLittleEndianBuffer(
        data.com_paras.c_id.chip_id_high,
        outBuffer,
        sizeof(data.com_paras.c_id.chip_id_low));
    fcsStatus = data.status;
    return true;
}

bool FcsCommunication::sigmaTeardown(uint32_t sessionId, int32_t &fcsStatus)
{
    Logger::log("Calling sigmaTeardown with session ID: "
        + std::to_string(static_cast<int32_t>(sessionId)));
    intel_fcs_dev_ioctl data = {};
    data.com_paras.tdown.teardown = true;
    data.com_paras.tdown.sid = sessionId;

    if (!sendIoctl(&data, INTEL_FCS_DEV_PSGSIGMA_TEARDOWN))
    {
        return false;
    }
    fcsStatus = data.status;
    return true;
}

bool FcsCommunication::createAttestationSubkey(
    std::vector<uint8_t> &inBuffer,
    std::vector<uint8_t> &outBuffer,
    int32_t &fcsStatus)
{
    Logger::log("Calling createAttestationSubkey");
    outBuffer.resize(ATTESTATION_SUBKEY_RSP_MAX_SZ);

    intel_fcs_dev_ioctl data = {};
    data.com_paras.subkey.resv.resv_word = 0;
    data.com_paras.subkey.cmd_data = (char*)inBuffer.data();
    data.com_paras.subkey.cmd_data_sz = inBuffer.size();
    data.com_paras.subkey.rsp_data = (char*)outBuffer.data();
    data.com_paras.subkey.rsp_data_sz = outBuffer.size();

    if (!sendIoctl(&data, INTEL_FCS_DEV_ATTESTATION_SUBKEY)
        || data.com_paras.subkey.rsp_data_sz > ATTESTATION_SUBKEY_RSP_MAX_SZ)
    {
        return false;
    }

    outBuffer.resize(data.com_paras.subkey.rsp_data_sz);
    fcsStatus = data.status;
    return true;
}

bool FcsCommunication::getMeasurement(
    std::vector<uint8_t> &inBuffer,
    std::vector<uint8_t> &outBuffer,
    int32_t &fcsStatus)
{
    Logger::log("Calling getMeasurement");
    outBuffer.resize(ATTESTATION_MEASUREMENT_RSP_MAX_SZ);

    intel_fcs_dev_ioctl data = {};
    data.com_paras.measurement.resv.resv_word = 0;
    data.com_paras.measurement.cmd_data = (char*)inBuffer.data();
    data.com_paras.measurement.cmd_data_sz = inBuffer.size();
    data.com_paras.measurement.rsp_data = (char*)outBuffer.data();
    data.com_paras.measurement.rsp_data_sz = outBuffer.size();

    if (!sendIoctl(&data, INTEL_FCS_DEV_ATTESTATION_MEASUREMENT)
        || data.com_paras.measurement.rsp_data_sz > ATTESTATION_MEASUREMENT_RSP_MAX_SZ)
    {
        return false;
    }

    outBuffer.resize(data.com_paras.measurement.rsp_data_sz);
    fcsStatus = data.status;
    return true;
}

bool FcsCommunication::getAttestationCertificate(
    uint8_t certificateRequest,
    std::vector<uint8_t> &outBuffer,
    int32_t &fcsStatus)
{
    Logger::log("Calling getAttestationCertificate");
    outBuffer.resize(ATTESTATION_CERTIFICATE_RSP_MAX_SZ);

    intel_fcs_dev_ioctl data = {};
    data.com_paras.certificate.c_request = certificateRequest;
    data.com_paras.certificate.rsp_data = (char*)outBuffer.data();
    data.com_paras.certificate.rsp_data_sz = outBuffer.size();

    if (!sendIoctl(&data, INTEL_FCS_DEV_ATTESTATION_GET_CERTIFICATE)
        || data.com_paras.certificate.rsp_data_sz > ATTESTATION_CERTIFICATE_RSP_MAX_SZ)
    {
        return false;
    }

    outBuffer.resize(data.com_paras.certificate.rsp_data_sz);
    fcsStatus = data.status;
    return true;
}

bool FcsCommunication:: mailboxGeneric(
    uint32_t commandCode,
    std::vector<uint8_t> &inBuffer,
    std::vector<uint8_t> &outBuffer,
    int32_t &fcsStatus)
{
    Logger::log("Calling mailbox generic command with code: " + std::to_string(commandCode));
    outBuffer.resize(MBOX_SEND_RSP_MAX_SZ);

    intel_fcs_dev_ioctl data = {};
    data.com_paras.mbox_send_cmd.mbox_cmd = commandCode;
    data.com_paras.mbox_send_cmd.urgent = 0;
    data.com_paras.mbox_send_cmd.cmd_data = (char*)inBuffer.data();
    data.com_paras.mbox_send_cmd.cmd_data_sz = inBuffer.size();
    data.com_paras.mbox_send_cmd.rsp_data = (char*)outBuffer.data();
    data.com_paras.mbox_send_cmd.rsp_data_sz = outBuffer.size();

    if (!sendIoctl(&data, INTEL_FCS_DEV_MBOX_SEND)
        || data.com_paras.mbox_send_cmd.rsp_data_sz > MBOX_SEND_RSP_MAX_SZ)
    {
        return false;
    }
    Logger::log("Received data from mailbox. Bytes: " + std::to_string(data.com_paras.mbox_send_cmd.rsp_data_sz));
    outBuffer.resize(data.com_paras.mbox_send_cmd.rsp_data_sz);
    fcsStatus = data.status;
    return true;
}
