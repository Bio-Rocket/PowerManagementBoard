/**
 ******************************************************************************
 * File Name          : TelemetryTask.cpp
 * Description        : Primary telemetry task, default task for the system.
 ******************************************************************************
*/
#include "TelemetryTask.hpp"
#include "GPIO.hpp"
#include "SystemDefines.hpp"
#include "PMBProtocolTask.hpp"
#include "FlightTask.hpp"

/**
 * @brief Constructor for TelemetryTask
 */
TelemetryTask::TelemetryTask() : Task(TELEMETRY_TASK_QUEUE_DEPTH_OBJS)
{
    loggingDelayMs = TELEMETRY_DEFAULT_LOGGING_RATE_MS;
    numNonFlashLogs_ = 0;
    numNonControlLogs_ = 0;
}

/**
 * @brief Initialize the TelemetryTask
 */
void TelemetryTask::InitTask()
{
    // Make sure the task is not already initialized
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize telemetry task twice");

    BaseType_t rtValue =
        xTaskCreate((TaskFunction_t)TelemetryTask::RunTask,
            (const char*)"TelemetryTask",
            (uint16_t)TELEMETRY_TASK_STACK_DEPTH_WORDS,
            (void*)this,
            (UBaseType_t)TELEMETRY_TASK_RTOS_PRIORITY,
            (TaskHandle_t*)&rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "TelemetryTask::InitTask() - xTaskCreate() failed");
}

/**
 * @brief Instance Run loop for the Telemetry Task, runs on scheduler start as long as the task is initialized.
 * @param pvParams RTOS Passed void parameters, contains a pointer to the object instance, should not be used
 */
void TelemetryTask::Run(void* pvParams)
{
    while (1) {
        //Process all commands in queue this cycle
        Command cm;
        while (qEvtQueue->Receive(cm))
            HandleCommand(cm);

        osDelay(loggingDelayMs);
        RunLogSequence();
    }
}

/**
 * @brief Handles a command from the command queue
 * @param cm Command to handle
 */
void TelemetryTask::HandleCommand(Command& cm)
{
    //Switch for the GLOBAL_COMMAND
    switch (cm.GetCommand()) {
    case TELEMETRY_CHANGE_PERIOD: {
        loggingDelayMs = (uint16_t)cm.GetTaskCommand();
    break;
    }
    default:
        SOAR_PRINT("TelemetryTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Runs a full logging sample/send sequence.
 *        can assume this is called with a period of loggingDelayMs
 */
void TelemetryTask::RunLogSequence()
{
    // Flight State
    FlightTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)FT_REQUEST_TRANSMIT_STATE));

    // Heartbeat Status (limited to every 2 seconds)
    if (++numNonControlLogs_ >= (TELEMETRY_HEARTBEAT_TIMER_PERIOD_MS / loggingDelayMs)) {
        numNonControlLogs_ = 0;
        WatchdogTask::Inst().SendCommand(Command(TASK_SPECIFIC_COMMAND, HB_STATUS_SEND));
    }

    // GPIO
    SendVentDrainStatus();

    // Other Sensors
    RequestSample();
    RequestTransmit();

    // Request Log to Flash
//    if(++numNonFlashLogs_ >= (PERIOD_BETWEEN_FLASH_LOGS_MS / loggingDelayMs)) {
//        numNonFlashLogs_ = 0;
//        RequestLogToFlash();
//    }
}

/**
 * @brief Poll requests to each sensor
 */
void TelemetryTask::RequestSample()
{
    // Battery
    BatteryTask::Inst().SendCommand(Command(REQUEST_COMMAND, BATTERY_REQUEST_NEW_SAMPLE));

    // Barometer
    BarometerTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)BARO_REQUEST_NEW_SAMPLE));

    // IMU
    IMUTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)IMU_REQUEST_NEW_SAMPLE));

    // Pressure Transducer
    //PressureTransducerTask::Inst().SendCommand(Command(REQUEST_COMMAND, PT_REQUEST_NEW_SAMPLE));
}

/**
 * @brief Requests transmit to each sensor
 */
void TelemetryTask::RequestTransmit()
{
    // Battery
    BatteryTask::Inst().SendCommand(Command(REQUEST_COMMAND, BATTERY_REQUEST_TRANSMIT));

    // Barometer
    BarometerTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)BARO_REQUEST_TRANSMIT));

    // IMU
    IMUTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)IMU_REQUEST_TRANSMIT));

    // Pressure Transducer
    //PressureTransducerTask::Inst().SendCommand(Command(REQUEST_COMMAND, PT_REQUEST_TRANSMIT));

    // GPS
    GPSTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)GPS_REQUEST_TRANSMIT));
}

/**
 * @brief Requests log to flash for each sensor that supports it
 */
void TelemetryTask::RequestLogToFlash()
{
    // Barometer
    BarometerTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)BARO_REQUEST_FLASH_LOG));

    // IMU
    IMUTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)IMU_REQUEST_FLASH_LOG));

    // GPS
    GPSTask::Inst().SendCommand(Command(REQUEST_COMMAND, (uint16_t)GPS_REQUEST_FLASH_LOG));
}

/**
 * @brief Sends the vent and drain status to the RCU
 */
void TelemetryTask::SendVentDrainStatus()
{
    Proto::TelemetryMessage teleMsg;
    teleMsg.set_source(Proto::Node::NODE_DMB);
    teleMsg.set_target(Proto::Node::NODE_RCU);
    Proto::CombustionControlStatus gpioMsg;
    gpioMsg.set_drain_open(GPIO::Drain::IsOpen());
    gpioMsg.set_vent_open(GPIO::Vent::IsOpen());
    gpioMsg.set_mev_open(GPIO::_MAIN_ENGINE_VALVE::IsOpen());
    teleMsg.set_combustionControlStatus(gpioMsg);

    EmbeddedProto::WriteBufferFixedSize<DEFAULT_PROTOCOL_WRITE_BUFFER_SIZE> writeBuffer;
    teleMsg.serialize(writeBuffer);

    // Send the control message
    DMBProtocolTask::SendProtobufMessage(writeBuffer, Proto::MessageID::MSG_TELEMETRY);
}
