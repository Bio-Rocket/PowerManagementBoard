/**
 ******************************************************************************
 * File Name          : DMBProtocolTask.hpp
 * Description        : Protocol task, specific to DMB
 ******************************************************************************
*/
#ifndef SOAR_PMBPROTOCOL_HPP_
#define SOAR_PMBPROTOCOL_HPP_
#include "ProtocolTask.hpp"
#include "Task.hpp"
#include "SystemDefines.hpp"
#include "UARTTask.hpp"

/* Enums ------------------------------------------------------------------*/

/* Class ------------------------------------------------------------------*/
class PMBProtocolTask : public ProtocolTask
{
public:
    static PMBProtocolTask& Inst() {
        static PMBProtocolTask inst;
        return inst;
    }

    void InitTask();

    static void SendProtobufMessage(EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE>& writeBuffer, Proto::MessageID msgId)
    {
        Inst().ProtocolTask::SendProtobufMessage(writeBuffer, msgId);
    }

protected:
    static void RunTask(void* pvParams) { PMBProtocolTask::Inst().Run(pvParams); } // Static Task Interface, passes control to the instance Run();

    // These handlers will receive a buffer and size corresponding to a decoded message
    void HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    void HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer);
    
    // Member variables

private:
    PMBProtocolTask();        // Private constructor
    PMBProtocolTask(const PMBProtocolTask&);                        // Prevent copy-construction
    PMBProtocolTask& operator=(const PMBProtocolTask&);            // Prevent assignment
};

#endif    // SOAR_PMBPROTOCOL_HPP_
