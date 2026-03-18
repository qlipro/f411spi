/*
 * printf.h
 *
 *  Created on: 2026年3月5日
 *      Author: jt
 */

#ifndef INC_PRINTF_H_
#define INC_PRINTF_H_

#include <stdint.h>
#include "LCD.h"

void LCD_ShowChar(uint8_t x, uint8_t y, char ch, uint16_t color, uint16_t bg_color, const ASCIIFont *font);
void LCD_InitContext(uint8_t y, uint8_t count_of_line, const ASCIIFont *font, uint16_t text_color, uint16_t bg_color);
void LCD_Printf(const char *format, ...);
void LCD_NewLine(void);
void LCD_SyncCursor(void);
void LCD_RefreshDisplay(void);
void LCD_FormFeed(void);
#endif /* INC_PRINTF_H_ */
