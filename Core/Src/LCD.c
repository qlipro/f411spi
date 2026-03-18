/*
 * LCD.c
 *
 *  Created on: Feb 18, 2026
 *      Author: jt
 */

#include "LCD.h"
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "spi.h"
#include "font.h"
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>


volatile uint8_t dma_busy = 0;
volatile uint8_t dma_clear_active = 0;

// LCD.c 文件开头添加静态变量
static uint8_t dma_buffer[2][X_MAX_PIXEL * 8 * 2] __attribute__((aligned(4)));  // 4字节对齐
static volatile uint16_t dma_current_line = 0;
static volatile uint16_t dma_total_lines = 0;

// 在LCD.c开头添加
static uint8_t lcd_buffer[2048];  // 复用缓冲区，避免重复分配
//static uint8_t lcd_buffer_ready = 0;

//static LCD_State_t lcd_state = LCD_IDLE;
//static uint16_t lcd_clear_color;
//static uint16_t lcd_current_line;
//static uint32_t lcd_line_start_time;
//static uint8_t lcd_buffer[X_MAX_PIXEL*8*2];

volatile DMA_Operation_t dma_current_op = DMA_OP_NONE;
static uint16_t dma_op_total = 0;
static uint16_t dma_op_current = 0;

 void (*dma_callback)(void) = NULL;  // 用户回调

void LCD_WriteCmd(uint8_t cmd) {
    DC_SET0();// 1. 拉低 D/CX 引脚 (命令模式)
    CS_SET0();// 2. 拉低 CSX 引脚 (使能)
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);// 3. 通过SPI或并行总线发送命令字节
    CS_SET1();// 4. 拉高 CSX 引脚
}

void LCD_WriteData(uint8_t data) {
    // 用户需要实现:
	DC_SET1();// 1. 拉高 D/CX 引脚 (数据模式)
	CS_SET0();// 2. 拉低 CSX 引脚 (使能)
	HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);// 3. 通过SPI或并行总线发送数据字节
	CS_SET1();// 4. 拉高 CSX 引脚
}

void LCD_WriteData2(uint16_t data) {
    // 用户需要实现:
	DC_SET1();// 1. 拉高 D/CX 引脚 (数据模式)
	CS_SET0();// 2. 拉低 CSX 引脚 (使能)
//	SPI_WriteByte(data);
	uint8_t d[2];
	 d[0] = data>>8;
	 d[1] = data;
	HAL_SPI_Transmit(&hspi2, d, 2, HAL_MAX_DELAY);// 3. 通过SPI或并行总线发送数据字节
	CS_SET1();// 4. 拉高 CSX 引脚
}

void LCD_WriteDataMulti(uint8_t *data, uint16_t len) {
	DC_SET1();     // 数据模式
	CS_SET0();     // 使能片选
    HAL_SPI_Transmit(&hspi2, data, len, HAL_MAX_DELAY);
    CS_SET1();
}




void LCD_Init(void){
	 // 硬件复位
	RST_SET0();
	HAL_Delay(10);
	RST_SET1();
	HAL_Delay(5);

	 // 软件复位
	LCD_WriteCmd(0x01);
	HAL_Delay(120);

	// 退出睡眠
	LCD_WriteCmd(0x11);
	HAL_Delay(120);

	// RGB565模式
	LCD_WriteCmd(0x3A);
	LCD_WriteData(0x05);

	// 默认方向 (0度)
	LCD_WriteCmd(0x36);
//	LCD_WriteData(0x00);
	LCD_WriteData(0xC0);
	// 常用配置：
	// 0x00 = 正常，RGB
	// 0x08 = 正常，BGR（某些模块需要）
	// 0xC0 = 旋转180度
	// 0x60 = 旋转90度
	// 0xA0 = 旋转270度

//	可选
//	//设置帧率
//	LCD_WriteCmd(0xB1);
//	LCD_WriteData(0x05);
//	LCD_WriteData(0x3A);
//	LCD_WriteData(0x3A);
//
//	//电源控制
//	LCD_WriteCmd(0xC0);
//	LCD_WriteData(0xA8);
//	LCD_WriteData(0x08);
//	LCD_WriteData(0x84);
//
//	LCD_WriteCmd(0xC1);
//	LCD_WriteData(0xC5);
//
//	LCD_WriteCmd(0xC2);
//	LCD_WriteData(0x0A);
//	LCD_WriteData(0x00);
//
//	LCD_WriteCmd(0xC3);
//	LCD_WriteData(0x8A);
//	LCD_WriteData(0x26);
//
//	LCD_WriteCmd(0xC4);
//	LCD_WriteData(0x8A);
//	LCD_WriteData(0xEE);
//
//	//VCOM
//	LCD_WriteCmd(0xC5);
//	LCD_WriteData(0x0F);
//	LCD_WriteData(0x0F);
//
//	LCD_WriteCmd(0xE0);
//	uint8_t gamma1[] = {0x02,0x1C,0x07,0x12,0x37,
//                      0x32,0x29,0x2D,0x29,0x25,
//			          0x2B,0x39,0x00,0x01,0x03,0x10};
//	LCD_WriteDataMulti(gamma1, 16);
//
//	LCD_WriteCmd(0xE1);
//	uint8_t gamma2[] = {0x03,0x1D,0x07,0x06,0x2E,
//            			0x2C,0x29,0x2D,0x2E,0x2E,
//						0x37,0x3F,0x00,0x00,0x02,0x10};
//	LCD_WriteDataMulti(gamma2, 16);

	// 开启显示
	LCD_WriteCmd(0x29);
	HAL_Delay(50);



}

//static
 void set_window(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1) {
	LCD_WriteCmd(0x2A);
	LCD_WriteData(0x00);
	LCD_WriteData(x0);
	LCD_WriteData(0x00);
	LCD_WriteData(x1);

	LCD_WriteCmd(0x2B);
	LCD_WriteData(0x00);
	LCD_WriteData(y0);
	LCD_WriteData(0x00);
	LCD_WriteData(y1);

	LCD_WriteCmd(0x2C);
}

void set_pixel(uint8_t x, uint8_t y, uint16_t color){
	set_window(x,y,x,y);
	LCD_WriteData2(color);
}

uint8_t* LCD_ReadLine(uint8_t y, uint8_t h, uint8_t *rgb565_out){

	LCD_WriteCmd(0x2A);
	LCD_WriteData(0x00);
	LCD_WriteData(0);
	LCD_WriteData(0x00);
	LCD_WriteData(X_MAX_PIXEL-1);

	LCD_WriteCmd(0x2B);
	LCD_WriteData(0x00);
	LCD_WriteData(y);
	LCD_WriteData(0x00);
	LCD_WriteData(y+h-1);

	LCD_WriteCmd(0x2E);

    // 批量读取
    DC_SET1();
    CS_SET0();

    uint8_t dummy = 0xFF;
    HAL_SPI_Transmit(&hspi2, &dummy, 1, HAL_MAX_DELAY);

	uint8_t buf[h*X_MAX_PIXEL*3];
	HAL_SPI_Receive(&hspi2, buf, h*X_MAX_PIXEL*3, HAL_MAX_DELAY);
	CS_SET1();

	for(uint8_t i=0;i<12 * 128 *2;i+=2){
	    uint8_t r6 = buf[i*3 + 0];
	    uint8_t g6 = buf[i*3 + 1];
	    uint8_t b6 = buf[i*3 + 2];
		uint16_t t = ((r6 & 0xF8) << 8) |    // R: 取高5位
                     ((g6 & 0xFC) << 3) |    // G: 取高6位
					 ((b6 & 0xF8) >> 3);
		rgb565_out[i] = t>>8;
		rgb565_out[i+1] = t&0xFF;
	}
//	return 1;
}

// LCD.c 中添加统一的DMA启动函数
static HAL_StatusTypeDef LCD_StartDMA(DMA_Operation_t op,
                                       uint8_t *data,
                                       uint16_t size,
                                       void (*callback)(void)) {
    // 等待DMA空闲
    uint32_t timeout = HAL_GetTick() + 1000;
    while(dma_clear_active) {
        if(HAL_GetTick() > timeout) {
            // 超时，强制复位
            CS_SET1();
            dma_clear_active = 0;
            dma_current_op = DMA_OP_NONE;
            break;
        }
        HAL_Delay(1);
    }

    dma_current_op = op;
    dma_callback = callback;
    dma_clear_active = 1;

    DC_SET1();
    CS_SET0();
    return HAL_SPI_Transmit_DMA(&hspi2, data, size);
}



void LCD_Clear(uint16_t color){
	set_window(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
	DC_SET1();     // 数据模式

	uint8_t d[X_MAX_PIXEL*8*2];
	uint8_t high = color >> 8;
	uint8_t low = color & 0xFF;
	for(uint16_t i=0; i<sizeof(d); i+=2) {
		d[i] = high;
		d[i+1] = low;
	}

	CS_SET0();
	for(uint16_t y = 0; y < Y_MAX_PIXEL/8; y++){
	HAL_SPI_Transmit(&hspi2, d, X_MAX_PIXEL*8*2, 100);
	}
	CS_SET1();
}

void LCD_Clear_DMA(uint16_t color){
	// 如果上一次DMA清屏还在进行，等待完成
	while(dma_clear_active) {
		HAL_Delay(1);  // 短暂延时，避免死循环



	}
dma_current_op = DMA_OP_CLEAR;
		dma_callback = NULL;
	set_window(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
	DC_SET1();     // 数据模式

	// 准备两个缓冲区（只做一次）
	uint8_t high = color >> 8;
	uint8_t low = color & 0xFF;

	for(uint16_t i = 0; i < X_MAX_PIXEL * 8 * 2; i += 2) {
		dma_buffer[0][i] = high;
		dma_buffer[0][i+1] = low;
		dma_buffer[1][i] = high;
		dma_buffer[1][i+1] = low;
	}

    // 初始化行计数器
    dma_current_line = 0;
    dma_total_lines = Y_MAX_PIXEL / 8;
    dma_clear_active = 1;
    dma_busy = 1;  // 使用原有的dma_busy标志

	CS_SET0();
	if(HAL_SPI_Transmit_DMA(&hspi2, dma_buffer[0], X_MAX_PIXEL * 8 * 2) != HAL_OK) {
		// 如果启动失败，恢复状态
		CS_SET1();
		dma_clear_active = 0;
		dma_busy = 0;
		dma_current_op = DMA_OP_NONE;
	}
}

/**
 * @brief  DMA传输完成回调处理（由spi.c调用）
 */
void LCD_DMA_Continue(void) {
    if(!dma_clear_active) return;  // 不是DMA清屏模式

    dma_current_line++;

    if(dma_current_line < dma_total_lines) {
            // 还有行需要发送，继续下一行（交替使用缓冲区）
            uint8_t *next_buffer = dma_buffer[dma_current_line % 2];
            if(HAL_SPI_Transmit_DMA(&hspi2, next_buffer, X_MAX_PIXEL * 8 * 2) != HAL_OK) {
                        // 如果启动失败，终止清屏
                        CS_SET1();
                        dma_clear_active = 0;
                        dma_busy = 0;
                    }
    }else {
		// 所有行发送完成
		CS_SET1();  // 最后才释放片选
		dma_clear_active = 0;
		dma_busy = 0;
	}
}

/**
 * @brief  查询DMA清屏是否完成
 * @return 1: 进行中, 0: 已完成
 */
uint8_t LCD_IsDMABusy(void) {
    return dma_clear_active;
}

/**
 * @brief  等待DMA清屏完成（如果需要同步）
 */
void LCD_WaitForDMA(void) {
    while(dma_clear_active) {
        HAL_Delay(1);
    }
}


//void LCD_Clear_NonBlocking(uint16_t color) {
//    if(lcd_state != LCD_IDLE) return;  // 忙
//
//    set_window(0, 0, X_MAX_PIXEL-1, Y_MAX_PIXEL-1);
//    DC_SET1();
//
//    uint8_t high = color >> 8;
//    uint8_t low = color & 0xFF;
//    for(uint16_t i=0; i<sizeof(lcd_buffer); i+=2) {
//        lcd_buffer[i] = high;
//        lcd_buffer[i+1] = low;
//    }
//
//    lcd_clear_color = color;
//    lcd_current_line = 0;
//    lcd_state = LCD_CLEARING;
//    lcd_line_start_time = HAL_GetTick();
//
//    CS_SET0();  // 片选使能
//}
//
//void LCD_Process(void) {
//    if(lcd_state != LCD_CLEARING) return;
//
//    // 检查行超时
//    if(HAL_GetTick() - lcd_line_start_time > 500) {
//        lcd_state = LCD_ERROR;
//        CS_SET1();
//        return;
//    }
//
//    // 检查SPI是否空闲
//    if(HAL_SPI_GetState(&hspi2) != HAL_SPI_STATE_READY) {
//        return;  // SPI忙，等待
//    }
//
//    // 发送当前行
//    if(HAL_SPI_Transmit(&hspi2, lcd_buffer, sizeof(lcd_buffer), 0) == HAL_OK) {
//        lcd_current_line++;
//        lcd_line_start_time = HAL_GetTick();
//
//        if(lcd_current_line >= Y_MAX_PIXEL/8) {
//            // 完成
//            CS_SET1();
//            lcd_state = LCD_IDLE;
//        }
//    } else {
//        // 立即返回错误，但由SPI状态机处理
//    }
//}
//
//uint8_t LCD_IsBusy(void) {
//    return (lcd_state != LCD_IDLE);
//}
//
//void LCD_Reset(void) {
//    CS_SET1();
//    lcd_state = LCD_IDLE;
//    lcd_current_line = 0;
//
//    // 复位SPI
//    HAL_SPI_DeInit(&hspi2);
//    HAL_Delay(10);
//    HAL_SPI_Init(&hspi2);
//
//    // 重新初始化LCD
//    LCD_Init();
//}
// 在LCD.c文件开头添加以下定义（在现有定义之后）

void LCD_ShowString_DMA(uint8_t x, uint8_t y, const char *str,
                        uint16_t color, const ASCIIFont *font
//						,void (*callback)(void)
                       ){
    while(dma_clear_active) HAL_Delay(1);  // 等待DMA空闲


    uint8_t char_width = font->w;
       uint8_t char_height = font->h;
       uint8_t bytes_per_char = char_height;
       uint16_t len = strlen(str);
       uint16_t total_width = char_width * len;


    // 限制最大显示长度
    if (total_width > X_MAX_PIXEL) {
        len = X_MAX_PIXEL / char_width;
        total_width = char_width * len;
    }

    // 使用静态缓冲区
    static uint8_t lcd_buffer[2048];
    uint16_t buffer_size = total_width * char_height * 2;
    if (buffer_size > sizeof(lcd_buffer)) {
        return;
    }

    // 获取字模数据指针
        uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

        uint8_t high = color >> 8;
        uint8_t low = color & 0xFF;
        uint16_t idx = 0;

    // 填充缓冲区
        for (uint8_t row = 0; row < char_height; row++) {
            for (uint16_t col = 0; col < total_width; col++) {
                uint8_t char_idx = col / char_width;
                uint8_t char_col = col % char_width;

                char ch = str[char_idx];
                if (ch < 32 || ch > 126) ch = '?';

                uint8_t row_data = font_data[ch - 32][row];

                if (char_width == 6) {
                    lcd_buffer[idx++] = (row_data & (0x40 >> char_col)) ? high : 0;
                    lcd_buffer[idx++] = (row_data & (0x40 >> char_col)) ? low : 0;
                } else {
                    lcd_buffer[idx++] = (row_data & (0x80 >> char_col)) ? high : 0;
                    lcd_buffer[idx++] = (row_data & (0x80 >> char_col)) ? low : 0;
                }
            }
        }

        set_window(x, y, x + total_width - 1, y + char_height - 1);

//    dma_current_op = DMA_OP_STRING;
//    dma_callback = callback;
//    dma_op_total = 1;  // 一次传输完成
//    dma_clear_active = 1;
//
//    DC_SET1();
//    CS_SET0();
//    HAL_SPI_Transmit_DMA(&hspi2, lcd_buffer, total_width * char_height * 2);
        LCD_StartDMA(DMA_OP_STRING, lcd_buffer, buffer_size, NULL);
}

void LCD_ShowString(uint8_t x, uint8_t y, const char *str, uint16_t color, const ASCIIFont *font) {
    uint8_t char_width = font->w;
    uint8_t char_height = font->h;
    uint8_t bytes_per_char = char_height;  // 每个字符的字节数等于高度
    uint16_t len = strlen(str);
    uint16_t total_width = char_width * len;

    // 限制最大显示长度，避免缓冲区溢出
        if (total_width > X_MAX_PIXEL) {
            len = X_MAX_PIXEL / char_width;
            total_width = char_width * len;
        }

    // 缓冲区大小
    uint16_t buffer_size = total_width * char_height * 2;

    if (buffer_size > sizeof(lcd_buffer)) {
           return;  // 缓冲区不足
       }

    // 设置显示窗口
    set_window(x, y, x + total_width - 1, y + char_height - 1);

    // 获取字模数据指针
    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;
    uint16_t idx = 0;

// 填充缓冲区
    for (uint8_t row = 0; row < char_height; row++) {
        for (uint16_t col = 0; col < total_width; col++) {
            uint8_t char_idx = col / char_width;
            uint8_t char_col = col % char_width;

            char ch = str[char_idx];
            if (ch < 32 || ch > 126) ch = '?';

            uint8_t row_data = font_data[ch - 32][row];

            if (char_width == 6) {
                lcd_buffer[idx++] = (row_data & (0x40 >> char_col)) ? high : 0;
                lcd_buffer[idx++] = (row_data & (0x40 >> char_col)) ? low : 0;
            } else {
                lcd_buffer[idx++] = (row_data & (0x80 >> char_col)) ? high : 0;
                lcd_buffer[idx++] = (row_data & (0x80 >> char_col)) ? low : 0;
            }
        }
    }
    // 发送缓冲区
    DC_SET1();
    CS_SET0();
    HAL_SPI_Transmit(&hspi2, lcd_buffer, buffer_size, HAL_MAX_DELAY);
    CS_SET1();
}

/**
 * @brief 显示字符串（支持背景色）
 * @param x        起始x坐标
 * @param y        起始y坐标
 * @param str      要显示的字符串
 * @param fg_color 前景色（字体颜色）
 * @param bg_color 背景色
 * @param font     字体结构体指针
 */
void LCD_ShowStringBG(uint8_t x, uint8_t y, const char *str,
                    uint16_t fg_color, uint16_t bg_color,
                    const ASCIIFont *font) {
    uint8_t char_width = font->w;
    uint8_t char_height = font->h;
    uint8_t bytes_per_char = char_height;
    uint16_t len = strlen(str);
    uint16_t total_width = char_width * len;

    // 限制最大显示长度
    if (total_width > X_MAX_PIXEL) {
        len = X_MAX_PIXEL / char_width;
        total_width = char_width * len;
    }

    // 使用静态缓冲区
    static uint8_t lcd_buffer[2048];
    uint16_t buffer_size = total_width * char_height * 2;
    if (buffer_size > sizeof(lcd_buffer)) {
        return;
    }

    // 设置窗口
    set_window(x, y, x + total_width - 1, y + char_height - 1);

    // 获取字模数据
    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

    uint8_t fg_high = fg_color >> 8;
    uint8_t fg_low = fg_color & 0xFF;
    uint8_t bg_high = bg_color >> 8;
    uint8_t bg_low = bg_color & 0xFF;
    uint16_t idx = 0;

    // 行扫描填充
    for (uint8_t row = 0; row < char_height; row++) {
        for (uint16_t col = 0; col < total_width; col++) {
            uint8_t char_idx = col / char_width;
            uint8_t char_col = col % char_width;

            char ch = str[char_idx];
            if (ch < 32 || ch > 126) ch = '?';

            uint8_t row_data = font_data[ch - 32][row];

            // 根据字体宽度选择掩码
            uint8_t mask;
            if (char_width == 6) {
                mask = 0x40 >> char_col;  // 6位字体
            } else {
                mask = 0x80 >> char_col;  // 8位字体
            }

            // 判断是前景还是背景
            if (row_data & mask) {
                // 前景色（字符）
                lcd_buffer[idx++] = fg_high;
                lcd_buffer[idx++] = fg_low;
            } else {
                // 背景色
                lcd_buffer[idx++] = bg_high;
                lcd_buffer[idx++] = bg_low;
            }
        }
    }

    // 发送缓冲区
    DC_SET1();
    CS_SET0();
    HAL_SPI_Transmit(&hspi2, lcd_buffer, buffer_size, HAL_MAX_DELAY);
    CS_SET1();
}

void LCD_ShowStringBG_DMA(uint8_t x, uint8_t y, const char *str,
                          uint16_t fg_color, uint16_t bg_color,
                          const ASCIIFont *font
//                          ,void (*callback)(void)
						  ) {
    uint8_t char_width = font->w;
    uint8_t char_height = font->h;
    uint8_t bytes_per_char = char_height;
    uint16_t len = strlen(str);
    uint16_t total_width = char_width * len;

    // 限制最大显示长度
    if (total_width > X_MAX_PIXEL) {
        len = X_MAX_PIXEL / char_width;
        total_width = char_width * len;
    }

    // 使用静态缓冲区（注意：与无背景色版本共用同一个缓冲区）
    static uint8_t dma_string_buffer[2048] __attribute__((aligned(4)));
    uint16_t buffer_size = total_width * char_height * 2;
    if (buffer_size > sizeof(dma_string_buffer)) {

        return;
    }

    // 获取字模数据指针
    uint8_t (*font_data)[bytes_per_char] = (uint8_t (*)[bytes_per_char])font->chars;

    uint8_t fg_high = fg_color >> 8;
    uint8_t fg_low = fg_color & 0xFF;
    uint8_t bg_high = bg_color >> 8;
    uint8_t bg_low = bg_color & 0xFF;
    uint16_t idx = 0;

    // 填充缓冲区（带背景色）
    for (uint8_t row = 0; row < char_height; row++) {
        for (uint16_t col = 0; col < total_width; col++) {
            uint8_t char_idx = col / char_width;
            uint8_t char_col = col % char_width;

            char ch = str[char_idx];
            if (ch < 32 || ch > 126) ch = '?';

            uint8_t row_data = font_data[ch - 32][row];

            uint8_t mask;
            if (char_width == 6) {
                mask = 0x40 >> char_col;
            } else {
                mask = 0x80 >> char_col;
            }

            if (row_data & mask) {
                dma_string_buffer[idx++] = fg_high;
                dma_string_buffer[idx++] = fg_low;
            } else {
                dma_string_buffer[idx++] = bg_high;
                dma_string_buffer[idx++] = bg_low;
            }
        }
    }

    // 设置窗口
    set_window(x, y, x + total_width - 1, y + char_height - 1);

    // 启动DMA传输
    LCD_StartDMA(DMA_OP_STRING_BG, dma_string_buffer, buffer_size, NULL);
}

// 清除指定区域（比全屏清快得多）
void LCD_ClearArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint16_t color) {
    set_window(x, y, x + w - 1, y + h - 1);

    uint16_t pixels = w * h;
    uint8_t high = color >> 8;
    uint8_t low = color & 0xFF;

    // 使用较小的局部缓冲区
    uint8_t area_buf[64];  // 一次最多清32个像素
    for (uint16_t i = 0; i < sizeof(area_buf); i += 2) {
        area_buf[i] = high;
        area_buf[i+1] = low;
    }

    DC_SET1();
    CS_SET0();

    // 分批发送
    while (pixels > 0) {
        uint16_t send = (pixels > 32) ? 32 : pixels;
        HAL_SPI_Transmit(&hspi2, area_buf, send * 2, HAL_MAX_DELAY);
        pixels -= send;
    }

    CS_SET1();
}
