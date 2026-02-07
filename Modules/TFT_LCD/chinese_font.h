#ifndef __CHINESE_FONT_H
#define __CHINESE_FONT_H

#include <stdint.h>

// ==================== 16x16 中文字库索引 ====================
typedef enum {
    ZH_SHU = 0,   // 输
    ZH_RU,        // 入
    ZH_MI,        // 密
    ZH_MA,        // 码
    ZH_CUO,       // 错
    ZH_WU,        // 误
    ZH_WEN,       // 温
    ZH_DU,        // 度
	ZH_SHI, //湿
	ZH_GUANG, //光
	ZH_ZHAO, //照
	ZH_FENG, //风
	ZH_SHAN, //扇
	ZH_CHUANG, //窗
	ZH_LIANG, //帘
	ZH_MO, //模
	ZH_SHI2, //式
	ZH_SHI3, //实
	ZH_SHI4 , //时
	ZH_JIAN ,// 间
    ZH_COUNT
} ChineseIndex_16x16;

// 16x16：每个汉字 2 行（每行16字节）
extern const uint8_t chinese_fonts_16x16[][16];

// 24x24：PC2LCD2002（C51格式）常见输出为 16+16+16+16+8 = 72字节
typedef struct {
    uint8_t b0[16];
    uint8_t b1[16];
    uint8_t b2[16];
    uint8_t b3[16];
    uint8_t b4[8];
} typFNT_GB24_C51;

extern const typFNT_GB24_C51 chinese_fonts_24x24[];

#endif


