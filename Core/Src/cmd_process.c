/*
 * cmd_process.c
 *
 *  Created on: 2026年3月18日
 *      Author: jt
 */

#include "cmd_process.h"
#include "stdint.h"
#include "aht20.h"
#include "string.h"
#include "encoder.h"
#include "stdio.h"
#include "printf.h"
#include "drv8833.h"
#include "tim.h"


uint8_t motor_tag = 0;
uint8_t servo_tag = 0;

/*掩码从低位往高位填入，每位对应指令集合中的索引
 */

HAL_StatusTypeDef tag_check(){
	if(motor_tag == 0
			&& servo_tag ==0
//			&&other_tag==0
			){
		return HAL_OK;
	}
}

void tag_reset(){
	motor_tag = 0;
	servo_tag = 0;
}

void cmd_process(uint32_t mask){
	if (mask== 0x134){             //温度读取
		float temperature, humidity;
		AHT20_Read(&temperature, &humidity);
		char message_temp[30];
		sprintf(message_temp, "temperature: %d", (uint8_t)temperature);
		LCD_Printf("\n%s",message_temp);

	}else
	if (mask== 0x234){             //湿度读取
		float temperature, humidity;
		AHT20_Read(&temperature, &humidity);

		char message_hum[30];
		sprintf(message_hum, "humidity: %d%%", (uint8_t)humidity);
		LCD_Printf("\n%s",message_hum);
	}else
	if(mask == 0x124){
		motor_tag = 1;
	}else
	if(mask == 0x114){
		servo_tag = 1;
	}
	//else if for (mask)
}
