/*
 * cursor_global.c
 *
 *  Created on: 2026年3月6日
 *      Author: jt
 */

// cursor_global.c
#include "cursor.h"

// 定义全局变量（只在一处定义）
LCD_Context lcd_ctx = {
    .cursor_x = 0,
    .cursor_y = 0,
    .line_height = 0,
    .text_color = 0xFFFF,
    .bg_color = 0x0000,
    .width = X_MAX_PIXEL - 16,
    .height = 0,
    .font = NULL,
    .auto_wrap = 1,
    .start_y = 0,
    .line_count = 0,
	.bg_height = 0,
    .current_page = 0,
    .page_count = 1,
    .page_history_head = 0,
	.history ={{{0}}}
};

LCD_Context_Buffer lcd_c_buf = {
    .buff = {0},
    .buff_head_line = 0,
    .buff_head_pos = 0,
    .buff_end_line = 0,
    .buff_end_pos = 0,
    .buff_len = 0
};
