/**
  ******************************************************************************
  * File Name          : BMSTask.cpp
  *
  * Description        : Handles BMS functions for a single battery pack
  *                      Reads data
  *                      the thermocouples on the plumbing bay board. A
  *                      thread task is included that will wait for update and transmit
  *                      requests for reading and updating the temperature fields.
  ******************************************************************************
*/

#include "BMSTask.hpp"
#include "SystemDefines.hpp"
#include "data.h"
#include "BQ769x0_Driver.hpp"
#include "DebugTask.hpp"

BMSTask::BMSTask() : Task(TASK_BMS_QUEUE_DEPTH_OBJS) {}

void BMSTask::InitTask()
{
    SOAR_ASSERT(rtTaskHandle == nullptr, "Cannot initialize BMS task twice");

    BaseType_t rtValue = xTaskCreate(
        (TaskFunction_t)BMSTask::RunTask,
        (const char*)"BMSTask",
        TASK_BMS_STACK_DEPTH_WORDS,
        (void*)this,
        TASK_BMS_PRIORITY,
        &rtTaskHandle);

    SOAR_ASSERT(rtValue == pdPASS, "BMSTask::InitTask - xTaskCreate failed");
}

/**
 * @brief ThermocoupleTask run loop
 * @param pvParams Currently unused task context
 */
void BMSTask::Run(void * pvParams)
{
    while (1) {
        Command cm;

        //Wait forever for a command
        qEvtQueue->ReceiveWait(cm);

        //Process the command
        HandleCommand(cm);
    }
}

/**
 * @brief Handles a command
 * @param cm Command reference to handle
 */
void BMSTask::HandleCommand(Command& cm)
{
    //Switch for the GLOBAL_COMMAND
    switch (cm.GetCommand()) {
    case REQUEST_COMMAND: {
        HandleRequestCommand(cm.GetTaskCommand()); //Sends task specific request command to task request handler
        break;
    }
    case TASK_SPECIFIC_COMMAND: {
        break; //No task specific commands need
    }
    default:
        SOAR_PRINT("ThermocoupleTask - Received Unsupported Command {%d}\n", cm.GetCommand());
        break;
    }

    //No matter what we happens, we must reset allocated data
    cm.Reset();
}

/**
 * @brief Handles a Request Command
 * @param taskCommand The command to handle
 */
void ThermocoupleTask::HandleRequestCommand(uint16_t taskCommand)
{
    //Switch for task specific command within DATA_COMMAND
    switch (taskCommand) {
    case THERMOCOUPLE_REQUEST_NEW_SAMPLE: //Sample TC and store in class fields
    	SampleThermocouple();
        break;
    case THERMOCOUPLE_REQUEST_TRANSMIT: //Sending data to PI
        TransmitProtocolThermoData();
        break;
    case THERMOCOUPLE_REQUEST_DEBUG: //Output TC data
        ThermocoupleDebugPrint();
        break;
    default:
        SOAR_PRINT("UARTTask - Received Unsupported REQUEST_COMMAND {%d}\n", taskCommand);
        break;
    }
}

void BMSTask::SampleBMSData()
{

}
