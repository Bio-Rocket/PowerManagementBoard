/**
 ******************************************************************************
 * File Name          : DMBProtocolTask.hpp
 * Description        : Protocol task, specific to DMB
 ******************************************************************************
*/
#include "PMBProtocolTask.hpp"

#include "FlightTask.hpp"
#include "ReadBufferFixedSize.h"
#include "TelemetryTask.hpp"

/**
 * @brief Initialize the PMBProtocolTask
 */
void PMBProtocolTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize Protocol task twice");

    // Start the task
    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)PMBProtocolTask::RunTask,
            (const char*)"ProtocolTask",
            (uint16_t)TASK_PROTOCOL_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)PMB_PROTOCOL_TASK_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    //Ensure creation succeded
    SOAR_ASSERT(rtValue == pdPASS, "ProtocolTask::InitTask - xTaskCreate() failed");
}

/**
 * @brief Default constructor
 */
PMBProtocolTask::PMBProtocolTask() : ProtocolTask(
        Proto::Node::NODE_PMB,
        UART::Radio,
        UART_TASK_COMMAND_SEND_RADIO)
{
}

/**
 * @brief Handle a command message
 */
void PMBProtocolTask::HandleProtobufCommandMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::CommandMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, if they aren't as expected, do nothing
    if (msg.get_source() != Proto::Node::NODE_RCU || msg.get_target() != Proto::Node::NODE_PMB)
        return;

    // If the message does not have a PMB command, do nothing
    if (!msg.has_pmb_command())
        return;

    // SOAR_PRINT("PROTO-INFO: Received PMB Command Message\n");

    // Process the db command
    switch (msg.get_pmb_command().get_command_enum())
    {
    case Proto::DmbCommand::Command::RSC_ANY_TO_ABORT:
        FlightTask::Inst().SendCommand(Command(CONTROL_ACTION, (uint16_t)RSC_ANY_TO_ABORT));
        break;
    default:
        break;
    }

}

/**
 * @brief Handle a control message
 */
void PMBProtocolTask::HandleProtobufControlMesssage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{
    Proto::ControlMessage msg;
    msg.deserialize(readBuffer);

    // Verify the source and target nodes, if they aren't as expected, do nothing
    if (msg.get_source() != Proto::Node::NODE_RCU || msg.get_target() != Proto::Node::NODE_PMB)
        return;

    // Handle based on the message type
    if(msg.has_hb()) {
        // This is a heartbeat message, update the heartbeat
        // SOAR_PRINT("PROTO-INFO: Received Heartbeat Message\n");
        WatchdogTask::Inst().SendCommand(Command(HEARTBEAT_COMMAND, (uint16_t)RADIOHB_REQUEST));
    }
    else if(msg.has_ping()) {
        // This is a ping message, respond with an ack
        Proto::ControlMessage ackResponse;
        Proto::AckNack ack;
        ack.set_acking_msg_source(msg.get_source());
        ack.set_acking_sequence_num(msg.get_source_sequence_num());
        ackResponse.set_ack(ack);
        EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuf;
        ackResponse.serialize(writeBuf);
        PMBProtocolTask::SendProtobufMessage(writeBuf, Proto::MessageID::MSG_CONTROL);
    }
    else if(msg.has_sys_ctrl()) {
		// This is a system command, handle it
	    if (msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_FLASH_LOG_ENABLE)
	    {
            // TODO
	    }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_FLASH_LOG_DISABLE)
        {
            // TODO
        }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_RESET)
        {
			// This is a request to reset the system
            SOAR_ASSERT(false, "System reset requested!");
        }
        else if (msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_CRITICAL_FLASH_FULL_ERASE)
        {
            // This is a request that will erase all flash memory, and cause the flash task to stall!
            FlashTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, ERASE_ALL_FLASH));
        }
        else if(msg.get_sys_ctrl().get_sys_cmd() == Proto::SystemControl::Command::SYS_LOG_PERIOD_CHANGE)
        {
            uint32_t paramMs = msg.get_sys_ctrl().get_cmd_param();
            paramMs = (paramMs > 0xFFFF) ? 0xFFFE : paramMs;
            TelemetryTask::Inst().SendCommand(Command(TELEMETRY_CHANGE_PERIOD, paramMs));
        }
    }
}

/**
 * @brief Handle a telemetry message
 */
void PMBProtocolTask::HandleProtobufTelemetryMessage(EmbeddedProto::ReadBufferFixedSize<PROTOCOL_RX_BUFFER_SZ_BYTES>& readBuffer)
{

}
