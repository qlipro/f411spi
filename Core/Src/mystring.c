/*
 * mystring.c
 *
 *  Created on: 2026年3月16日
 *      Author: jt
 */
#include "mystring.h"
#include <string.h>


const char* charset[] = {
    "abcdefghijklmnopqrstuvwxyz",            // 模式0: 小写字母
    "ABCDEFGHIJKLMNOPQRSTUVWXYZ",            // 模式1: 大写字母
    "0123456789",                            // 模式2: 数字
	" !\"#$%&'()*+,-./:;<=>?@[\\]^_`{|}~",    // 模式3: 符号 (33个)
	"\b \t\r\n\v\f"                          // 模式4: 控制字符 (退格,空格,制表,回车,换行,垂直制表,换页
};

// 控制字符的预览显示（与charset[4]一一对应）
const char* ctrl_preview[] = {
	    "\\b",  // 退格
	    "[]",  // 空格
	    "\\t",  // 制表符
	    "\\r",  // 回车
	    "\\n",  // 换行
	    "\\v",  // 垂直制表
	    "\\f"   // 换页
};

// 模式名称
const char* mode_names[] = {
    "Mode: letters",
    "Mode: CAPS   ",
    "Mode: numbers",
    "Mode: symbols",
    "Mode: CTRL   "
};

const char* command[] = {
		"set>","cal>","page_to>","func>",NULL
};

const char* command_set[] = {
		"pwm>","uart>","BLK>",NULL
};

const char* command_func[] = {
		"servo>","motor>","aht>",NULL
};

const char* command_cal[] = {
		"1","2","3","4","5",
		"6","7","8","9","0",
		"+","-","*","/","(",
		")","^",">","\\b",NULL
};

const char* command_mode[] = {
		"on","off",NULL
};

const char* command_aht20[] = {
		"get_temp","get_hum",NULL
};


// 辅助宏：从位掩码中提取第level层的模式值
#define DECODE_MODE(mask, level)  (((mask) >> ((level) * 4)) & 0x0F)

// 位掩码与指令集的映射（按层级位置编码）
const ModeMaskToCommandSet mode_mask_to_cmdset[] = {
    // 单层映射
    {ENCODE_MODE(0, 0), command},
    {ENCODE_MODE(0, 1), command_set},
    {ENCODE_MODE(0, 2), command_cal},
    {ENCODE_MODE(0, 3), command_cal},
    {ENCODE_MODE(0, 4), command_func},

    // 两层映射
    {ENCODE_MODE(0, 1) | ENCODE_MODE(1, 1), command_mode},
	{ENCODE_MODE(0, 1) | ENCODE_MODE(1, 2), command_mode},
	{ENCODE_MODE(0, 1) | ENCODE_MODE(1, 3), command_mode},
    {ENCODE_MODE(0, 4) | ENCODE_MODE(1, 1), command_mode},
	{ENCODE_MODE(0, 4) | ENCODE_MODE(1, 2), command_mode},
	{ENCODE_MODE(0, 4) | ENCODE_MODE(1, 3), command_aht20},


    // 三层映射
//    {ENCODE_MODE(0, 0) | ENCODE_MODE(1, 5) | ENCODE_MODE(2, 1), command_cal},  // {0,5,1} -> command_cal

    // 可以继续添加更多映射...
};

const uint8_t mode_mask_count = sizeof(mode_mask_to_cmdset) / sizeof(mode_mask_to_cmdset[0]);

// 根据位掩码获取对应的指令集
const char** get_command_set_by_mask(uint32_t mask) {
    // 精确匹配
    for (int i = 0; i < mode_mask_count; i++) {
        if (mode_mask_to_cmdset[i].mode_mask == mask) {
            return mode_mask_to_cmdset[i].cmd_set;
        }
    }
};
