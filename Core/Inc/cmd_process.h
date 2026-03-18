/*
 * cmd_process.h
 *
 *  Created on: 2026年3月18日
 *      Author: jt
 */

#ifndef INC_CMD_PROCESS_H_
#define INC_CMD_PROCESS_H_

#include "stdint.h"
#include "main.h"

extern uint8_t motor_tag;
extern uint8_t servo_tag;

HAL_StatusTypeDef tag_check();
void tag_reset();
void cmd_process(uint32_t mask);

#endif /* INC_CMD_PROCESS_H_ */
