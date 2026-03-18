/*
 * cursor.c
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#include "cursor.h"
#include "LCD.h"
#include "spi.h"
#include "printf.h"



// ==================== 辅助函数 ====================

void LCD_GetLine(uint16_t line_index, char *buffer) {
    if (!buffer) return;

    if (line_index < lcd_ctx.history_count) {
        strcpy(buffer, lcd_ctx.history[lcd_ctx.current_page][line_index]);
    } else {
        buffer[0] = '\0';
    }
}

void LCD_ScrollUp(uint8_t lines) {
    if (!lcd_ctx.font || lines == 0) return;

    if (lcd_ctx.history_head + lines < lcd_ctx.history_count) {
        lcd_ctx.history_head += lines;
    } else {
        lcd_ctx.history_head = lcd_ctx.history_count > lines ?
                               lcd_ctx.history_count - lines : 0;
    }

    LCD_RefreshDisplay();
}

void LCD_GetCursorPos(uint16_t *line, uint8_t *pos) {
    if (line) *line = lcd_ctx.current_line;
    if (pos) *pos = lcd_ctx.current_line_pos;
}

uint8_t LCD_SetCursorPos(uint16_t line, uint8_t pos) {
    if (!lcd_ctx.font) return 0;

    if (line < lcd_ctx.history_count && pos <= MAX_CHARS_PER_LINE) {
        lcd_ctx.current_line = line;
        lcd_ctx.current_line_pos = pos;
        LCD_SyncCursor();
        LCD_RefreshDisplay();
        return 1;
    }
    return 0;
}

void LCD_CursorBlink(void){
	static uint32_t last_cursor_blink = 0;
	static uint8_t cursor_state = 0;
    if (HAL_GetTick() - last_cursor_blink > 500) {  // 500ms 闪烁一次
        last_cursor_blink = HAL_GetTick();
        cursor_state = !cursor_state;

        if (cursor_state) {
            // 显示光标：在光标位置画一条下划线
            LCD_ClearArea(lcd_ctx.cursor_x,
                         lcd_ctx.cursor_y + lcd_ctx.font->h - 1,
                         lcd_ctx.font->w, 1,
                         0xFFFF);  // 白色下划线
        } else {
            // 隐藏光标：用背景色覆盖下划线
            LCD_ClearArea(lcd_ctx.cursor_x,
                         lcd_ctx.cursor_y + lcd_ctx.font->h - 1,
                         lcd_ctx.font->w, 1,
                         lcd_ctx.bg_color);  // 背景色
        }
    }

}
