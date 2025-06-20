/**
  ******************************************************************************
  * @file           : SystemDefines.hpp
  * @brief          : Macros and wrappers
  ******************************************************************************
  *
  * Contains system wide macros, defines, and wrappers
  *
  ******************************************************************************
  */
#ifndef SOAR_MAIN_SYSTEM_DEFINES_H
#define SOAR_MAIN_SYSTEM_DEFINES_H

/* Environment Defines ------------------------------------------------------------------*/
//#define COMPUTER_ENVIRONMENT        // Define this if we're in Windows, Linux or Mac (not when flashing on DMB)

#ifdef COMPUTER_ENVIRONMENT
#define __CC_ARM
#endif

/* System Wide Includes ------------------------------------------------------------------*/
#include <cstdint>        // For uint32_t, etc.
#include <cstdio>        // Standard c printf, vsnprintf, etc.
#ifdef COMPUTER_ENVIRONMENT// COMPUTER -------------------------------------------------
#include <cassert>        // Standard c assert, not needed except on POSIX
#include <cstdlib>        // Standard c malloc, not needed except on POSIX
#endif

#include "cmsis_os.h"    // CMSIS RTOS definitions
#include "main_avionics.hpp"  // Main avionics definitions
#include "Utils.hpp"    // Utility functions
#include "stm32g0xx_hal.h"
#include "UARTDriver.hpp"

/* Specific Task Configuration  ------------------------------------------------------------------*/
constexpr uint32_t TELEMETRY_DEFAULT_LOGGING_RATE_MS = 100; // Default logging delay for telemetry task
constexpr uint32_t TELEMETRY_MINIMUM_LOG_PERIOD_MS = 20; // (1000/20 = 50hz) The minimum log period / max log rate


/* Task Definitions ------------------------------------------------------------------*/
/* - Lower priority number means lower priority task ---------------------------------*/

// FLIGHT PHASE
constexpr uint8_t FLIGHT_TASK_RTOS_PRIORITY = 4;            // Priority of the flight task
constexpr uint8_t FLIGHT_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the flight task queue
constexpr uint16_t FLIGHT_TASK_STACK_DEPTH_WORDS = 512;        // Size of the flight task stack

constexpr uint16_t FLIGHT_PHASE_DISPLAY_FREQ = 1000;    // Display frequency for flight phase information

// DMB PROTOCOL TASK
constexpr uint16_t PMB_PROTOCOL_TASK_PRIORITY = 4;     // Priority of DMB protocol task (Rx from Ground Systems)

// UART TASK
constexpr uint8_t UART_TASK_RTOS_PRIORITY = 3;            // Priority of the uart task
constexpr uint8_t UART_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the uart task queue
constexpr uint16_t UART_TASK_STACK_DEPTH_WORDS = 512;    // Size of the uart task stack

// DEBUG TASK
constexpr uint8_t TASK_DEBUG_PRIORITY = 1;            // Priority of the debug task
constexpr uint8_t TASK_DEBUG_QUEUE_DEPTH_OBJS = 10;        // Size of the debug task queue
constexpr uint16_t TASK_DEBUG_STACK_DEPTH_WORDS = 512;        // Size of the debug task stack

// TELEMETRY Task
constexpr uint8_t TELEMETRY_TASK_RTOS_PRIORITY = 2;            // Priority of the telemetry task
constexpr uint8_t TELEMETRY_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the telemetry task queue
constexpr uint16_t TELEMETRY_TASK_STACK_DEPTH_WORDS = 512;        // Size of the telemetry task stack

// BMS Task
constexpr uint8_t BMS_TASK_RTOS_PRIORITY = 2;            // Priority of the BMS task
constexpr uint8_t BMS_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the BMS task queue
constexpr uint16_t BMS_TASK_STACK_DEPTH_WORDS = 512;        // Size of the BMS task stack

// Fuel Gauge Task
constexpr uint8_t FUEL_GAUGE_TASK_RTOS_PRIORITY = 1;            // Priority of the fuel gauge task
constexpr uint8_t FUEL_GAUGE_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the fuel gauge task queue
constexpr uint16_t FUEL_GAUGE_TASK_STACK_DEPTH_WORDS = 512;        // Size of the fuel gauge task stack

// Charger Task
constexpr uint8_t CHARGER_TASK_RTOS_PRIORITY = 2;            // Priority of the charger task
constexpr uint8_t CHARGER_TASK_QUEUE_DEPTH_OBJS = 10;        // Size of the charger task queue
constexpr uint16_t CHARGER_TASK_STACK_DEPTH_WORDS = 512;        // Size of the charger task stack

// TODO: Turn state machine into a task perhaps

/* System Defines ------------------------------------------------------------------*/
/* - Each define / constexpr must have a comment explaining what it is used for     */
/* - Each define / constexpr must be all-caps. Prefer constexpr unless it's a string, or a calculation (eg. mathematical expression being more readable) */
// RTOS
constexpr uint8_t DEFAULT_QUEUE_SIZE = 10;                    // Default size of the queue
constexpr uint16_t MAX_NUMBER_OF_COMMAND_ALLOCATIONS = 100;    // Let's assume ~128B per allocation, 100 x 128B = 12800B = 12.8KB

// DEBUG
constexpr uint16_t DEBUG_TAKE_MAX_TIME_MS = 500;        // Max time in ms to take the debug semaphore
constexpr uint16_t DEBUG_SEND_MAX_TIME_MS = 500;        // Max time the assert fail is allowed to wait to send header and message to HAL
constexpr uint16_t DEBUG_PRINT_MAX_SIZE = 192;            // Max size in bytes of message print buffers

// ASSERT
constexpr uint16_t ASSERT_BUFFER_MAX_SIZE = 160;        // Max size in bytes of assert buffers (assume x2 as we have two message segments)
constexpr uint16_t ASSERT_SEND_MAX_TIME_MS = 250;        // Max time the assert fail is allowed to wait to send header and message to HAL (will take up to 2x this since it sends 2 segments)
constexpr uint16_t ASSERT_TAKE_MAX_TIME_MS = 500;        // Max time in ms to take the assert semaphore
constexpr UARTDriver* const DEFAULT_ASSERT_UART_DRIVER = UART::Debug;    // UART Handle that ASSERT messages are sent over

/* System Functions ------------------------------------------------------------------*/
//- Any system functions with an implementation here should be inline, and inline for a good reason (performance)
//- Otherwise the function may have a better place in main_avionics.cpp

// Assert macro, use this for checking all possible program errors eg. malloc success etc. supports a custom message in printf format
// This is our version of the stm32f4xx_hal_conf.h 'assert_param' macro with support for optional messages
// Example Usage: SOAR_ASSERT(ptr != 0, "Pointer on loop index %d is null!", index);
#define SOAR_ASSERT(expr, ...) ((expr) ? (void)0U : soar_assert_debug(false, (const char *)__FILE__, __LINE__, ##__VA_ARGS__)) 

// SOAR_PRINT macro, acts as an interface to the print function which sends a packet to the UART Task to print data
#define SOAR_PRINT(str, ...) (print(str, ##__VA_ARGS__))

/**
 * @brief Malloc inline function, wraps malloc for multi-platform support, asserts successful allocation
 * @param size Size of data to malloc in bytes
 * @return Returns the pointer to the allocated data
*/
inline uint8_t* soar_malloc(uint32_t size) {
#ifdef COMPUTER_ENVIRONMENT
    uint8_t* ret = (uint8_t*)malloc(size);
#else
    uint8_t* ret = (uint8_t*)pvPortMalloc(size);
#endif
    
    SOAR_ASSERT(ret, "soar_malloc failed");
    return ret;
}

/**
 * @brief Free inline function, wraps free for multi-platform support
 * @param ptr Pointer to the data to free
 */
inline void soar_free(void* ptr) {
#ifdef COMPUTER_ENVIRONMENT
    free(ptr);
#else
    vPortFree(ptr);
#endif
}

/* STM32 HAL C++ Wrappers ------------------------------------------------------------------*/


/* Other ------------------------------------------------------------------*/
// Override the new and delete operator to ensure heap4 is used for dynamic memory allocation
inline void* operator new(size_t size) { return soar_malloc(size); }
inline void operator delete(void* ptr) { soar_free(ptr); }

#endif // SOAR_MAIN_SYSTEM_DEFINES_H
