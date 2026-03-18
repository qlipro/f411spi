/*
 * LCD.h
 *
 *  Created on: Feb 18, 2026
 *      Author: jt
 */

#ifndef INC_LCD_H_
#define INC_LCD_H_

#include <stdint.h>
#include "font.h"
#include "cursor.h"

//typedef enum {
//    LCD_IDLE,
//    LCD_CLEARING,
//    LCD_ERROR
//} LCD_State_t;

// LCD.h 中定义DMA操作类型
typedef enum {
    DMA_OP_NONE = 0,
    DMA_OP_CLEAR,      // 清屏
    DMA_OP_STRING,     // 显示字符串
	DMA_OP_STRING_BG,  // 带背景色的字符串
    DMA_OP_IMAGE,      // 显示图片
    DMA_OP_MENU,        // 菜单绘制
    DMA_OP_WIFI_LINE        // 新增：WIFI行传输	,适配wifi.c
} DMA_Operation_t;




extern volatile uint8_t dma_clear_active;  // 正在DMA传输
extern volatile DMA_Operation_t dma_current_op;
extern void (*dma_callback)(void);

//extern LCD_State_t lcd_state;
//extern uint16_t lcd_clear_color;
extern volatile uint8_t dma_busy ;
extern volatile uint8_t dma_clear_active;  // 清屏进行中标志

#define RED  	0xF800
#define GREEN	0x07E0
#define BLUE 	0x001F
#define WHITE	0xFFFF
#define BLACK	0x0000
#define YELLOW  0xFFE0
#define GRAY0   0xEF7D   	//灰色0 3165 00110 001011 00101
#define GRAY1   0x8410      	//灰色1      00000 000000 00000
#define GRAY2   0x4208
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define ORANGE  0xFD20

#define RST_SET1()  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_SET)
#define RST_SET0()  HAL_GPIO_WritePin(RST_GPIO_Port, RST_Pin, GPIO_PIN_RESET)
#define DC_SET1()  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_SET)
#define DC_SET0()  HAL_GPIO_WritePin(DC_GPIO_Port, DC_Pin, GPIO_PIN_RESET)

//硬件NSS，SPI_NSS_HARD_OUTPUT模式自动控制CS引脚，不需要gpio模拟
#define CS_SET1()  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_SET)
#define CS_SET0()  HAL_GPIO_WritePin(CS_GPIO_Port, CS_Pin, GPIO_PIN_RESET)

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        160

void LCD_WriteCmd(uint8_t cmd);
void LCD_WriteData(uint8_t data);
void LCD_WriteData2(uint16_t data);
void LCD_Init(void);

void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void set_pixel(uint8_t x, uint8_t y, uint16_t color);

uint8_t* LCD_ReadLine(uint8_t y, uint8_t h, uint8_t *rgb565_out);

void LCD_Clear(uint16_t color);
void LCD_Clear_DMA(uint16_t color);
void LCD_DMA_Continue(void);
uint8_t LCD_IsDMABusy(void);
void LCD_WaitForDMA(void);
void LCD_ClearArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color);

//void LCD_Clear_NonBlocking(uint16_t color);
//void LCD_Process(void);

void LCD_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t color, const ASCIIFont *font);
void LCD_ShowStringBG(uint8_t x, uint8_t y, const char *str,
                    uint16_t fg_color, uint16_t bg_color,
                    const ASCIIFont *font);
void LCD_ShowString_DMA(uint8_t x, uint8_t y, const char *str,
                        uint16_t color, const ASCIIFont *font
//                        ,void (*callback)(void)
						);
void LCD_ShowStringBG_DMA(uint8_t x, uint8_t y, const char *str,
                          uint16_t fg_color, uint16_t bg_color,
                          const ASCIIFont *font
//                          ,void (*callback)(void)
						  );
#endif /* INC_LCD_H_ */
