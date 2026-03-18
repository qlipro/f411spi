/*
 * string.h
 *
 *  Created on: 2026年3月16日
 *      Author: jt
 */

#ifndef INC_MYSTRING_H_
#define INC_MYSTRING_H_

#include <stdint.h>
// 辅助宏：将第level层的模式值mode编码到位掩码中
#define ENCODE_MODE(level, mode)  ((mode) << ((level) * 4))  // 每层用4位，支持0-15

typedef struct {
    uint32_t mode_mask;           // 32位位掩码，表示每层级的模式值
    const char** cmd_set;          // 对应的指令集
} ModeMaskToCommandSet;
const char** get_command_set_by_mask(uint32_t mask) ;
// 字符集
extern const char* charset[];

// 控制字符的预览显示（与charset[4]一一对应）
extern const char* ctrl_preview[];

// 模式名称
extern const char* mode_names[];

extern const char* command[];

extern const char* command_set[];

extern const char* command_func[];

extern const char* command_cal[];

extern const char* command_mode[];

extern const char* command_aht20[];

#endif /* INC_MYSTRING_H_ */
