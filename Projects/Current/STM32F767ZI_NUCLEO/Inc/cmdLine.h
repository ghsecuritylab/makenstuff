/*
 * cmdLine.h
 *
 *  Created on: Sep 2, 2019
 *      Author: chris
 */

#ifndef INC_CMDLINE_H_
#define INC_CMDLINE_H_

#include "includes.h"

#define RX_BUFFER_SIZE			64

void cmdLineTask(void);
void cmdLineInit(void);
void USART_CharReception_Callback(void);

#endif /* INC_CMDLINE_H_ */
