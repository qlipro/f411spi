/*
 * cursor.h
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#ifndef INC_CURSOR_H_
#define INC_CURSOR_H_

#include "LCD.h"

#define X_MAX_PIXEL	        128
#define Y_MAX_PIXEL	        160
// 文本缓冲区配置
#define MAX_HISTORY_LINES      25      // 最大文本行数（包含隐藏行）
#define MAX_CHARS_PER_LINE  17      // 每行最大字符数 ( (128-16)/8 = 14 )
#define MAX_PAGES           15   // 最大页数


#define LEFT_MARGIN            8        // 左边距（显示区域起始X）
#define TOP_MARGIN             0        // 上边距（由用户设置）
// 显示上下文结构体
typedef struct {
    const ASCIIFont *font;      // 当前字体
    uint16_t text_color;        // 文字颜色
    uint16_t bg_color;          // 背景颜色

    uint8_t cursor_x;           // 当前光标X坐标
    uint8_t cursor_y;           // 当前光标Y坐标
    uint8_t line_height;        // 行高



    uint8_t start_y;            // 起始Y坐标
    uint8_t line_count;         // 行数
    uint8_t width;              // 显示区域宽度
    uint8_t height;             // 显示区域高度
	uint8_t auto_wrap;          // 自动换行标志
	uint8_t max_charsperline;
	uint8_t bg_height;

    // 历史记录区
    char history[MAX_PAGES][MAX_HISTORY_LINES][MAX_CHARS_PER_LINE + 1];  // +1 留个 '\0'


    // 页管理
	uint16_t current_page;        // 当前页号
	uint16_t page_count;          // 总页数
	uint16_t page_history_head;   // 当前页的历史记录起始行
	// 行管理（当前页内）
    uint16_t history_count;      // 历史记录总行数
    uint16_t history_head;       // 当前显示的第一行在历史记录中的索引
    uint16_t current_line;       // 当前正在编辑的行号
    // 当前行内的位置
    uint8_t current_line_pos;   // 当前行已写入的字符数



} LCD_Context;

typedef struct {
	char buff[MAX_HISTORY_LINES*(MAX_CHARS_PER_LINE + 1)];
	uint8_t buff_head_line;
    uint8_t buff_head_pos;
    uint8_t buff_end_line;
    uint8_t buff_end_pos;
    uint8_t buff_len;

} LCD_Context_Buffer;

extern LCD_Context lcd_ctx; //在cursor_global.c中统一初始化
extern LCD_Context_Buffer lcd_c_buf;

void LCD_GetLine(uint16_t line_index, char *buffer);
void LCD_ScrollUp(uint8_t lines);
void LCD_GetCursorPos(uint16_t *line, uint8_t *pos);
uint8_t LCD_SetCursorPos(uint16_t line, uint8_t pos);
void LCD_CursorBlink(void);

#endif /* INC_CURSOR_H_ */
