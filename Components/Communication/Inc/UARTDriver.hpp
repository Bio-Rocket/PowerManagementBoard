#ifndef SOAR_UART_DRIVER_HPP_
#define SOAR_UART_DRIVER_HPP_
/**
 ******************************************************************************
 * File Name          : UARTDriver.hpp
 * Description        : UART Driver
 * Author             : cjchanx (Chris)
 ******************************************************************************
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32g0xx_ll_usart.h"
#include "stm32g0xx_hal_rcc.h"
#include "stm32g0xx_ll_dma.h"
#include "cmsis_os.h"

/* UART Driver Instances ------------------------------------------------------------------*/
class UARTDriver;

namespace Driver {
	extern UARTDriver uart1;
	extern UARTDriver uart2;
}

/* UART Driver Aliases ------------------------------------------------------------------*/
namespace UART {
	constexpr UARTDriver* Debug = &Driver::uart1;
	constexpr UARTDriver* Conduit_DMB = &Driver::uart2;
}

/* UART Receiver Base Class ------------------------------------------------------------------*/
/**
 * @brief Any classes that are expected to receive using a UART driver
 *		  must derive from this base class and provide an implementation
 *		  for InterruptRxData
 */
class UARTReceiverBase
{
public:
	virtual void InterruptRxData(uint8_t errors) = 0;
};


/* UART Driver Class ------------------------------------------------------------------*/
/**
 * @brief This is a basic UART driver designed for Interrupt Rx and Polling Tx
 *	      based on the STM32 LL Library
 */
class UARTDriver
{
public:
	UARTDriver(USART_TypeDef* uartInstance) :
		kUart_(uartInstance),
		rxCharBuf_(nullptr),
		rxReceiver_(nullptr) {}

	// Polling Functions
	bool Transmit(uint8_t* data, uint16_t len);

	// Interrupt Functions
	bool ReceiveIT(uint8_t* charBuf, UARTReceiverBase* receiver);


	// Interrupt Handlers
	void HandleIRQ_UART(); // This MUST be called inside USARTx_IRQHandler

protected:
	// Helper Functions
	bool HandleAndClearRxError();
	bool GetRxErrors();


	// Constants
	USART_TypeDef* kUart_; // Stores the UART instance

	// Variables
	uint8_t* rxCharBuf_; // Stores a pointer to the buffer to store the received data
	UARTReceiverBase* rxReceiver_; // Stores a pointer to the receiver object
};



#endif // SOAR_UART_DRIVER_HPP_
