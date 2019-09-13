/*
 * uart2.c
 *
 *  Created on: Sep 2, 2019
 *      Author: chris
 */

#include "uart2.h"

#define UART2_RX_BUFFER_SIZE	128
#define UART2_QUEUE_LENGTH		1

uint8_t uart2_rxBufferA[UART2_RX_BUFFER_SIZE];
uint8_t uart2_rxBufferB[UART2_RX_BUFFER_SIZE];

volatile uint32_t uart2_receivedChars;
volatile uint32_t uart2_bufferReadyflag;
volatile uint32_t uart2_bufferLength;
volatile uint32_t responseFlag;

uint8_t *uart2_pBufferReadyforProcessing;
uint8_t *uart2_pBufferReadyForReception;

void uart2HandleContinuousReception(void);

void uart2SendData(uint8_t *String, uint32_t Size)
{
  uint32_t index = 0;
  uint8_t *pchar = String;
  /* Send characters one per one, until last char to be sent */
  for (index = 0; index < Size; index++)
  {
    /* Wait for TXE flag to be raised */
    while (!((READ_BIT(USART2->ISR, USART_ISR_TXE) == (USART_ISR_TXE)) ? 1UL : 0UL))
    {
    }
    /* Write character in Transmit Data register.
       TXE flag is cleared by writing data in TDR register */
    USART2->TDR = *pchar;
    pchar++;
  }
  /* Wait for TC flag to be raised for last char */
  while (!((READ_BIT(USART2->ISR, USART_ISR_TC) == (USART_ISR_TC)) ? 1UL : 0UL))
  {
  }
}

void uart2Task(void)
{
  /* Infinite loop */
	uart2HandleContinuousReception();
  /* USER CODE END CmdlineTask */
}
uint8_t quality;
void uart2CharReceptionCallback(void)
{
	uint8_t *ptemp;
	uint8_t c;
//	uint16_t heading;
//	uint16_t distance;


	if(responseFlag == 0){
		uart2_bufferLength = 7;
	}else{
		uart2_bufferLength = app.response.data_response_length;
	}

	/* Read Received character. RXNE flag is cleared by reading of RDR register */
	c =  (uint8_t)(READ_BIT(USART2->RDR, USART_RDR_RDR));
	uart2_pBufferReadyForReception[uart2_receivedChars++] = c;

	/* Checks if Buffer full indication has been set */
	if ((uart2_receivedChars >= uart2_bufferLength))
	{
		/* Swap buffers for next bytes to be received */
		ptemp = uart2_pBufferReadyforProcessing;
		uart2_pBufferReadyforProcessing = uart2_pBufferReadyForReception;
		uart2_pBufferReadyForReception = ptemp;
		if(responseFlag == 0){
			app.response.start_flag1 = uart2_pBufferReadyforProcessing[0];
			app.response.start_flag2 = uart2_pBufferReadyforProcessing[1];
			app.response.data_response_length = ((uart2_pBufferReadyforProcessing[5]<<26) | (uart2_pBufferReadyforProcessing[4]<<16) | (uart2_pBufferReadyforProcessing[3]<<8) | (uart2_pBufferReadyforProcessing[2]));
			app.response.send_mode = (uart2_pBufferReadyforProcessing[5]<<6);
			app.response.data_type = uart2_pBufferReadyforProcessing[6];
			responseFlag = 1;
		}else{
			if(app.currentLidarState == GET_INFO){
				app.info.model = uart2_pBufferReadyforProcessing[0];
				app.info.firmware_minor = uart2_pBufferReadyforProcessing[1];
				app.info.firmware_major = uart2_pBufferReadyforProcessing[2];
				app.info.hardware = uart2_pBufferReadyforProcessing[3];
				app.info.serial[0] = uart2_pBufferReadyforProcessing[4];
				app.info.serial[1] = uart2_pBufferReadyforProcessing[5];
				app.info.serial[2] = uart2_pBufferReadyforProcessing[6];
				app.info.serial[3] = uart2_pBufferReadyforProcessing[7];
				app.info.serial[4] = uart2_pBufferReadyforProcessing[8];
				app.info.serial[5] = uart2_pBufferReadyforProcessing[9];
				app.info.serial[6] = uart2_pBufferReadyforProcessing[10];
				app.info.serial[7] = uart2_pBufferReadyforProcessing[11];
				app.info.serial[8] = uart2_pBufferReadyforProcessing[12];
				app.info.serial[9] = uart2_pBufferReadyforProcessing[13];
				app.info.serial[10] = uart2_pBufferReadyforProcessing[14];
				app.info.serial[11] = uart2_pBufferReadyforProcessing[15];
				app.info.serial[12] = uart2_pBufferReadyforProcessing[16];
				app.info.serial[13] = uart2_pBufferReadyforProcessing[17];
				app.info.serial[14] = uart2_pBufferReadyforProcessing[18];
				app.info.serial[15] = uart2_pBufferReadyforProcessing[19];
				app.currentLidarState = LIDAR_IDLE;
				responseFlag = 0;
			}else if(app.currentLidarState == SCAN){
				quality = (uart2_pBufferReadyforProcessing[0] << 2);
//				app.currentLidarState = LIDAR_IDLE;
//				responseFlag = 0;
			//	responseFlag = 0;
//				if(quality > 20){
//					heading = (uart2_pBufferReadyforProcessing[2] << 8) | (uart2_pBufferReadyforProcessing[1] << 1);
//					heading = heading / 64;
//					if(heading < 360){
//						distance = (uart2_pBufferReadyforProcessing[4] << 8) | (uart2_pBufferReadyforProcessing[3] << 0);
//						distance = distance / 4;
//						app.lidar.quality[heading] = quality;
//						app.lidar.distance[heading] = distance;
//					}
//				}
			}
//			uart2_bufferReadyflag = 1;
		}
		uart2_receivedChars = 0;
	}
}

void uart2InitReception(void)
{
	uart2_pBufferReadyForReception 	= uart2_rxBufferA;
	uart2_pBufferReadyforProcessing = uart2_rxBufferB;

	uart2_receivedChars = 0;
	uart2_bufferReadyflag = 0;
	uart2_bufferLength = UART2_RX_BUFFER_SIZE;

	responseFlag = 0;

	WRITE_REG(USART2->ICR, USART_ICR_ORECF);	//ClearFlag;
	SET_BIT(USART2->CR1, USART_CR1_RXNEIE);		//Enable IT RXNE;
	SET_BIT(USART2->CR3, USART_CR3_EIE);		//Enable IT ERROR;
}

void uart2HandleContinuousReception(void)
{
	/* Checks if Buffer full indication has been set */
	if (uart2_bufferReadyflag != 0)
	{
		/* Reset indication */
		 uart2_bufferReadyflag = 0;

		/* Do something with the data */

//			responseFlag = 0;
	}else{

	}
}