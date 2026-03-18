/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "dma.h"
#include "i2c.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "LCD.h"
#include "aht20.h"
#include "stdio.h"
#include "6050.h"
#include "printf.h"
#include "cursor.h"
#include "mystring.h"
#include "encoder.h"
#include "cmd_process.h"
#include "drv8833.h"
#include "servo.h"

#include <string.h>
#include <inttypes.h>  // 确保包含这个头文件
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define COUNT_MID 20
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
static uint32_t last_process_time = 0;
uint32_t last_display1 = 0;
uint32_t last_display2 = 0;
char display_buf[16];  // 局部缓冲区，不占用全局内存
char uart_buffer[32];
float angle1 =0;
float angle2 =0;


static int16_t last_position = 0;
static uint32_t last_rotate_time = 0;
static uint8_t timer_active = 0;        // 定时器是否激活

static uint32_t last_check = 0;


static uint8_t mode = 0;
static uint8_t char_index = 0;
static uint8_t inpmode = 0;
static uint8_t cmd_index = 0;
static uint8_t cmd_tag = 0;
static uint8_t cal_tag = 0;
static uint8_t cmd_level = 0;
static uint32_t mask = 0;

// 预览显示区域的位置（在文本框上方）
#define PREVIEW_X   8
#define PREVIEW_Y   120
#define MODE_X      8
#define MODE_Y      25
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_SPI2_Init();
  MX_TIM1_Init();
  MX_I2C1_Init();
  MX_TIM4_Init();
  MX_ADC1_Init();
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  for(uint16_t i=0x0000;i<CYAN;i+=((0x01<<6)+0X01)){
  LCD_Clear_DMA(i);
  }LCD_WaitForDMA();
  LCD_Clear(CYAN);

  DRV8833_Init();
  int count = 0;
  int speed = 0;

  if(AHT20_Init(&hi2c1)==HAL_OK){;
  LCD_ShowString(8, 14, "AHT20 is ready", RED, &afont8x6);
  }
  HAL_Delay(20);

  Servo_InitAlphaTable();
  if(MPU6050_Init(&hi2c1) == HAL_OK) {
  LCD_ShowString(8, 6, "MPU6050 is ready", RED, &afont8x6);
  	// 	  OLED_DrawImage((128 - (lklkImg.w)) / 2, 0, &lklkImg, OLED_COLOR_NORMAL);
  	// 	  OLED_ShowFrame();
  	/* 启动第一次DMA读取 */
  HAL_Delay(100);
  MPU6050_StartDMARead();
  } else {
  //	while(1);
  }

  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_3);
  HAL_TIM_PWM_Start(&htim4, TIM_CHANNEL_4);
  HAL_TIM_Encoder_Start(&htim1, TIM_CHANNEL_ALL);
  last_process_time = HAL_GetTick();

  LCD_InitContext(55, 5, &afont12x8, WHITE, BLACK);

  LCD_Printf(" hello, A&B\n");


  // 显示模式提示
  LCD_ShowString(8, 25, "Mode: letters", GREEN, &afont8x6);

  // 显示初始预览
  char preview_buf[20];
  sprintf(preview_buf, "[%c]", charset[mode][char_index]);
  LCD_ShowString(PREVIEW_X, PREVIEW_Y, preview_buf, YELLOW, &afont12x8);

  uint16_t current_line;
  uint8_t current_pos;
  uint16_t past_line ;
  uint8_t past_pos;
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

//
//		char message_temp[30];
//		sprintf(message_temp, "temperature: %d ", (uint8_t)temperature);
//		LCD_ShowString_DMA(8,44,message_temp,BLUE,&afont8x6);
//		LCD_WaitForDMA();
			    //光标闪烁处理
      LCD_CursorBlink();

	    // 显示光标位置信息（调试用）

	    LCD_GetCursorPos(&current_line, &current_pos);

	    char pos_buf[30];
	    sprintf(pos_buf, "L:%d P:%d", current_line, current_pos);
	    LCD_ShowString(8,35, pos_buf, GREEN, &afont8x6);
	    if(current_pos!=past_pos || current_line!=past_line){
	    LCD_ClearArea(8, 35, 55, 8, CYAN);
	    LCD_ShowString(8,35, pos_buf, GREEN, &afont8x6);
	    past_line=current_line;
	    past_pos=current_pos;
	    }
	    // ===== 添加这行：显示当前 inpmode =====
	       char mode_buf[20];
	       sprintf(mode_buf, "inpmode:%d", inpmode);
	       LCD_ShowString(8, 145, mode_buf, RED, &afont8x6);


	    // ==================== 1. 读取编码器位置 ====================
	    int16_t current_position = (int16_t)__HAL_TIM_GET_COUNTER(&htim1)/4;
//	    char a[20];
//	    sprintf(a,"%02d",current_position);
//	    LCD_ShowString(8, 125, a, RED, &afont8x6);

	    // ==================== 2. 检测编码器旋转 ====================
	    if (current_position != last_position) {
	    	HAL_Delay(20);
	    	if (current_position != last_position) {
	        int8_t direction = (current_position > last_position) ? 1 : -1;
	        last_position = current_position;
	        if(inpmode==0){
	        // 更新字符索引
	        int len = strlen(charset[mode]);
	        char_index = (char_index + len + direction) % len;

	        // 清除之前的预览
	        LCD_ClearArea(PREVIEW_X, PREVIEW_Y,8*lcd_ctx.font->w, lcd_ctx.font->h, CYAN);


	        // 显示当前选中的字符预览
	        if (mode == 4) {
	            // 控制字符模式：显示预览文本
	            LCD_ShowString(PREVIEW_X, PREVIEW_Y,
	                          ctrl_preview[char_index],
	                          RED, &afont12x8);
	        } else {
	            // 普通字符模式：显示字符
	            char preview_str[4] = "[ ]";
	            preview_str[1] = charset[mode][char_index];
	            LCD_ShowString(PREVIEW_X, PREVIEW_Y, preview_str,
	                          YELLOW, &afont12x8);
	        }}
	        if(inpmode==1){

	        	if(motor_tag == 1){
	        		  count = __HAL_TIM_GET_COUNTER(&htim1);
	        		  if (count > 60000){
	        			  count = 0;
	        			  __HAL_TIM_SET_COUNTER(&htim1, 0);
	        		  }else if (count > COUNT_MID * 2){
	        			  count = COUNT_MID * 2;
	        			  __HAL_TIM_SET_COUNTER(&htim1, COUNT_MID * 2);
	        		  }

	        		  if (count < COUNT_MID){
	        			  speed = (COUNT_MID - count) * 100 / COUNT_MID;
	        			  DRV8833_Backward(speed);
	        		  }else{
	        			  speed = (count - COUNT_MID) * 100 / COUNT_MID;
	        			  DRV8833_Forward(speed);
	        		  }
	        	}

	        	if(servo_tag == 1){

	        	}

	        	if(tag_check()==HAL_OK){
	        	const char** current_list = NULL;
	        	current_list = get_command_set_by_mask(mask);

	        	    // 计算选定指令集的大小
	        	    int list_size = 0;
	        	    if (current_list) {
	        	        while (current_list[list_size] != NULL) {
	        	            list_size++;
	        	        }
	        	    }

	        	    // 更新索引
	        	    if (list_size > 0) {
	        	        cmd_index = (cmd_index + list_size + direction) % list_size;
	        	    }

	        	    // 清除预览区域
	        	    LCD_ClearArea(PREVIEW_X, PREVIEW_Y, 8*lcd_ctx.font->w, lcd_ctx.font->h, CYAN);

	        	    // 复制选中的指令到preview_cmd
	        	    if (current_list && list_size > 0) {

	        	        strcpy(preview_cmd, current_list[cmd_index]);

	        	    }
	        	LCD_ShowString(PREVIEW_X, PREVIEW_Y, preview_cmd,
	        		                          YELLOW, &afont12x8);
	        	char index_buf[20];
	        		       sprintf(index_buf, "0x%" PRIx32, mask);
//	        		       sprintf(index_buf, "%lu", (unsigned long)mask);
	        		       LCD_ClearArea(88, 145, 24, 8, CYAN);
	        		       LCD_ShowString(88, 145, index_buf, RED, &afont8x6);
	        	}
	        }
	        if(inpmode==2){

	        	LCD_ClearArea(lcd_ctx.cursor_x,
	        	                         lcd_ctx.cursor_y + lcd_ctx.font->h - 1,
	        	                         lcd_ctx.font->w, 1,
	        	                         lcd_ctx.bg_color);

	        	// 模式2：移动光标
	        	    uint16_t current_line;
	        	    uint8_t current_pos;
	        	    LCD_GetCursorPos(&current_line, &current_pos);

					LCD_Context_Buffer lcd_c_buf = {
							.buff = {0},
							.buff_head_line = current_line,
							.buff_head_pos = current_pos,
							.buff_end_line = current_line,
							.buff_end_pos = current_pos,
							.buff_len = 0
					};//初始化暂存文本
					uint8_t end_pos = current_pos;  // 保存旧位置用于判断
	        	    // 左右旋转移动光标
	        	    if (direction > 0) {


	        	        // 右旋：光标右移
	        	        if (current_pos < lcd_ctx.max_charsperline) {
//	        	        	lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = ' ';
	        	            current_pos++;
	        	            if(current_pos>end_pos){end_pos=current_pos;}
//	        	            lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';
	        	        } else {
	        	            // 如果到行尾，换到下一行开头
	        	            if (current_line < lcd_ctx.history_count - 1) {
	        	                current_line++;
	        	                current_pos = 0;
	        	            }

	        	        }


	        	        //更新暂存文本数组，不更新显示
	        	        if(current_line==lcd_c_buf.buff_end_line && current_pos>lcd_c_buf.buff_end_pos){
	        	        	lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = ' ';
	        	        	lcd_c_buf.buff[lcd_c_buf.buff_len]=' ';
	        	        	lcd_c_buf.buff_len++;
	        	        	lcd_c_buf.buff_end_pos++;
	        	        	lcd_c_buf.buff[lcd_c_buf.buff_len]= '\0';
	        	        }
	        	        if(current_line>lcd_c_buf.buff_end_line){

	        	        	lcd_c_buf.buff_len += lcd_ctx.max_charsperline - lcd_c_buf.buff_end_pos + 1;
	        	        	lcd_c_buf.buff_end_line++;
	        	        	lcd_c_buf.buff_end_pos=0;
	        	        	lcd_c_buf.buff[lcd_c_buf.buff_len]= '\0';
	        	        }
//	        	        lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos] = '\0';
	        	    } else {
	        	        // 左旋：光标左移
	        	        if (current_pos > 0) {
	        	            current_pos--;
	        	        } else {
	        	            // 如果到行首，换到上一行末尾
	        	            if (current_line > 0) {
	        	                current_line--;
	        	                current_pos = strlen(lcd_ctx.history[lcd_ctx.current_page][current_line]);
	        	            }else {
	        	                // ===== 新增：如果已经是最顶行且是第一列 =====
	        	                // 检查是否还有上一行历史记录
	        	                if (lcd_ctx.history_head > 0) {
	        	                    // history_head 上移一行
	        	                    lcd_ctx.history_head--;

	        	                    // 当前行不变（还是第一行）
	        	                    current_line = 0;

	        	                    // 获取新显示的第一行的内容，将光标移到该行末尾
	        	                    current_pos = strlen(lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.history_head]);

	        	                    // 刷新显示
	        	                    LCD_RefreshDisplay();

//	        	                    // 可选：显示提示信息
//	        	                    LCD_ClearArea(PREVIEW_X, PREVIEW_Y, 3*lcd_ctx.font->w, lcd_ctx.font->h, CYAN);
//	        	                    LCD_ShowString(PREVIEW_X, PREVIEW_Y, "Scroll Up", WHITE, &afont12x8);
	        	                }
	        	            }
	        	        }

	        	        //更新暂存文本数组，不更新显示
	        	        if(current_line == lcd_c_buf.buff_head_line && current_pos<lcd_c_buf.buff_head_pos){
	        	        	memmove(&lcd_c_buf.buff[1],&lcd_c_buf.buff[0],lcd_c_buf.buff_len + 1);
	        	        	lcd_c_buf.buff[0] = lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos];
	        	        	lcd_c_buf.buff_len++;
	        	        	lcd_c_buf.buff_head_pos--;
	        	        }
	        	        if(current_line < lcd_c_buf.buff_head_line){
	        	        	memmove(&lcd_c_buf.buff[1],&lcd_c_buf.buff[0],lcd_c_buf.buff_len + 1);
							lcd_c_buf.buff[0] = lcd_ctx.history[lcd_ctx.current_page][lcd_ctx.current_line][lcd_ctx.current_line_pos];
							lcd_c_buf.buff_len++;
							lcd_c_buf.buff_head_line--;
							lcd_c_buf.buff_head_pos = lcd_ctx.max_charsperline;
	        	        }

	        	    }

	        	    // 更新光标位置
	        	    lcd_ctx.current_line = current_line;
	        	    lcd_ctx.current_line_pos = current_pos;
	        	    LCD_SyncCursor();
//	        	    LCD_RefreshDisplay();


	        }

	        // 更新计时器
	        last_rotate_time = HAL_GetTick();
	        timer_active = 1;  // 启动800ms计时
	    }
	    }
	    // ==================== 3. 检测模式切换按钮 ====================
	    // PA10 短按切换模式
	    if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
	        HAL_Delay(20);  // 消抖

	        if (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
	            // 等待按钮释放
	        	tag_reset();
	        	DRV8833_Brake();
	            uint32_t pressTime = 0;
	            int longPressHandled = 0;  // 标记是否已经处理过长按
	            uint32_t startTime = HAL_GetTick();  // 记录开始时间
	            while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_10) == GPIO_PIN_RESET) {
	                HAL_Delay(10);
	                pressTime = HAL_GetTick() - startTime;

					// 检测长按（例如1000ms = 1秒）
					if (pressTime >= 800 && !longPressHandled) {
						longPressHandled = 1;
						LCD_RefreshDisplay();// 确保光标位置正确
				        LCD_SyncCursor();
						// ===== 长按操作：切换输入模式 =====

						 inpmode = (inpmode + 1) % 3;

						// 显示提示信息
						LCD_ClearArea(PREVIEW_X, PREVIEW_Y,8*lcd_ctx.font->w, lcd_ctx.font->h, CYAN);
						LCD_ShowString(8, 130, "Input Mode Changed", YELLOW, &afont8x6);
						HAL_Delay(500);
						LCD_ClearArea(8, 130, 120, 8, CYAN);

						if(inpmode==1){
							LCD_Printf("\n");
						}
					}
	            }
	            if (!longPressHandled) {

	            // === 切换模式 ===
	            mode = (mode + 1) % 5;
	            char_index = 0;

	            // === 更新显示 ===

	            // 清除模式显示
	            LCD_ClearArea(MODE_X, MODE_Y, 80, 10, lcd_ctx.bg_color);

	            // 显示新模式名称
	            LCD_ShowString(MODE_X, MODE_Y, mode_names[mode],
	                          GREEN, &afont8x6);

	            // 清除预览区域
	            LCD_ClearArea(PREVIEW_X, PREVIEW_Y, 3*lcd_ctx.font->w, lcd_ctx.font->h, lcd_ctx.bg_color);

	            // 显示新模式的第一个字符预览
	            if (mode == 4) {
	                LCD_ShowString(PREVIEW_X, PREVIEW_Y,
	                              ctrl_preview[0], RED, &afont12x8);
	            } else {
	                char preview_str[4] = "[ ]";
	                preview_str[1] = charset[mode][0];
	                LCD_ShowString(PREVIEW_X, PREVIEW_Y, preview_str,
	                              YELLOW, &afont12x8);
	            }


	            // 重置计时器
	            last_rotate_time = HAL_GetTick();
//	            timer_active = 1;
	            }
	        }
	    }

	    // ==================== 4. 800ms超时检测 ====================
	    // 如果旋转后800ms没有操作，自动输出当前字符
	    if (timer_active && (HAL_GetTick() - last_rotate_time > 800)) {
	    	if (inpmode == 0){
				if (mode == 4) {
					// 控制字符模式：执行控制功能
					char ctrl_ch = charset[4][char_index];

					// 执行控制字符
					LCD_Printf("%c", ctrl_ch);


				} else {
					// 普通字符模式：输出当前字符
					char ch = charset[mode][char_index];
					LCD_Printf("%c", ch);


				}
	    	}
	    	if (inpmode == 1){
	    		if(tag_check()==HAL_OK){
					if(cal_tag == 0){
	//	    		cmd_mode_index [cmd_level]= cmd_index;
					mask|=ENCODE_MODE(cmd_level,cmd_index+1);
					}
					if(strcmp(preview_cmd, "\\b") == 0){
						LCD_Printf("\b");
					}
					else{LCD_Printf("%s",preview_cmd);}
					if(strcmp(preview_cmd, ">") == 0){
						cal_tag = 0;
						cmd_tag++;
					}

					if(cal_tag == 0){
					cmd_tag++;
					cmd_level++;
					}

					if(cmd_tag == 3){
						LCD_Printf(">>");
						//执行指令
						cmd_process(mask);

						LCD_Printf("\n");
						cmd_tag = 0;
						cal_tag = 0;
						mask = 0;
						cmd_level = 0;
					}
	    		}
	    	}
	        timer_active = 0;  // 停止计时


	    }
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 8;
  RCC_OscInitStruct.PLL.PLLN = 100;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
