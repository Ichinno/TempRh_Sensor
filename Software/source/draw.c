/******************************************************************************
 * Copyright (C) 2021, 
 *
 *  
 *  
 *  
 *  
 *
 ******************************************************************************/
 
/******************************************************************************
 ** @file draw.c
 **
 ** @brief Source file for draw functions
 **
 ** @author MADS Team 
 **
 ******************************************************************************/

/******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdio.h>
#include <string.h>
#include "epd.h"
#include "base_types.h"
#include <stdint.h>   // for int16_t, uint8_t, etc.
#include <math.h>     // 如果需要fabs之类，但也可不用

/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                            
 ******************************************************************************/
#define WIDTH 152
#define HEIGHT 152
#define BYTES_PER_ROW (WIDTH / 8)

/******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/******************************************************************************
 * Local type definitions ('typedef')                                         
 ******************************************************************************/

/******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/******************************************************************************
 * Local variable definitions ('static')                                      *
 ******************************************************************************/
// 简单的字体数据（5x7 字体，每个字符占 5 列）
static const unsigned char FONT_5X7[62][5] = {
    {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
    {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
    {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
    {0x22, 0x41, 0x49, 0x49, 0x36}, // 3
    {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
    {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
    {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
    {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
    {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
    {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
    {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
    {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
    {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
    {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
    {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
    {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
    {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
    {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
    {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
    {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
    {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
    {0x7F, 0x02, 0x04, 0x02, 0x7F}, // M
    {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
    {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
    {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
    {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
    {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
    {0x26, 0x49, 0x49, 0x49, 0x32}, // S
    {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
    {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
    {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
    {0x7F, 0x20, 0x18, 0x20, 0x7F}, // W
    {0x63, 0x14, 0x08, 0x14, 0x63}, // X
    {0x03, 0x04, 0x78, 0x04, 0x03}, // Y
    {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    {0x20, 0x54, 0x54, 0x54, 0x78}, // a
    {0x7F, 0x48, 0x44, 0x44, 0x38}, // b
    {0x38, 0x44, 0x44, 0x44, 0x20}, // c
    {0x38, 0x44, 0x44, 0x48, 0x7F}, // d
    {0x38, 0x54, 0x54, 0x54, 0x18}, // e
    {0x08, 0x7E, 0x09, 0x01, 0x02}, // f
    {0x18, 0xA4, 0xA4, 0xA4, 0x7C}, // g
    {0x7F, 0x08, 0x04, 0x04, 0x78}, // h
    {0x00, 0x44, 0x7D, 0x40, 0x00}, // i
    {0x40, 0x80, 0x84, 0x7D, 0x00}, // j
    {0x7F, 0x10, 0x28, 0x44, 0x00}, // k
    {0x00, 0x41, 0x7F, 0x40, 0x00}, // l
    {0x7C, 0x04, 0x18, 0x04, 0x78}, // m
    {0x7C, 0x08, 0x04, 0x04, 0x78}, // n
    {0x38, 0x44, 0x44, 0x44, 0x38}, // o
    {0xFC, 0x18, 0x24, 0x24, 0x18}, // p
    {0x18, 0x24, 0x24, 0x18, 0xFC}, // q
    {0x7C, 0x08, 0x04, 0x04, 0x08}, // r
    {0x48, 0x54, 0x54, 0x54, 0x20}, // s
    {0x04, 0x3F, 0x44, 0x40, 0x20}, // t
    {0x3C, 0x40, 0x40, 0x20, 0x7C}, // u
    {0x1C, 0x20, 0x40, 0x20, 0x1C}, // v
    {0x3C, 0x40, 0x30, 0x40, 0x3C}, // w
    {0x44, 0x28, 0x10, 0x28, 0x44}, // x
    {0x9C, 0xA0, 0xA0, 0xA0, 0x7C}, // y
    {0x44, 0x64, 0x54, 0x4C, 0x44}  // z
};

// 15 个字符：'0'~'9' '.' '°' '%' '-' ':'
// 每字符 40 行、每行 1 字节 => 8 像素宽、40 像素高
// 8×40 加粗字形，共16个字符（0..9, '.', '°', '%', '-', ':', 'C'）。
// 每个字符40字节，这里每行写10个byte，共4行。
static const unsigned char FONT_40PX[16][40] =
{
    // [0] => '0'
    {
        0x3C,0x7E,0xE7,0xC3,0xC3,0xC3,0xC3,0xE7,0x7E,0x7E,
        0xFF,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,
        0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xFF,
        0x7E,0x7E,0xE7,0xC3,0xC3,0xC3,0xC3,0xE7,0x7E,0x3C
    },
    // [1] => '1'
    {
        0x18,0x3C,0x7E,0x7E,0x3C,0x18,0x18,0x18,0x18,0x18,
        0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
        0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,
        0x18,0x7E,0x7E,0x3C,0x18,0x18,0x00,0x00,0x00,0x00
    },
    // [2] => '2'
    {
        0x3C,0x7E,0xE7,0xC3,0x03,0x07,0x0E,0x1C,0x38,0x70,
        0x60,0xC0,0xC0,0xC0,0xC1,0xC3,0xC3,0xC3,0xC3,0xC3,
        0xC3,0xC3,0xC3,0xC6,0xCC,0xD8,0xF0,0xE0,0xC0,0xFF,
        0xFF,0xFF,0x00,0x00,0xC3,0xE7,0x7E,0x3C,0x00,0x00
    },
    // [3] => '3'
    {
        0x3C,0x7E,0xE7,0xC3,0x03,0x07,0x3E,0x07,0x03,0xC3,
        0xC3,0xC3,0xE7,0x7E,0x3C,0x7E,0xE7,0xC3,0xC3,0x03,
        0x07,0x3E,0x07,0x03,0xC3,0xC3,0xE7,0x7E,0x3C,0x00,
        0x7C,0xEE,0xC6,0x06,0x0E,0x1C,0x38,0x70,0xE0,0xC0
    },
    // [4] => '4'
    {
        0x0E,0x1E,0x3E,0x36,0x66,0x66,0xC6,0xC6,0xFE,0xFE,
        0x06,0x0E,0x0C,0x1C,0x18,0x38,0x30,0x70,0x60,0xE0,
        0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xFE,0xFE,
        0x18,0x18,0x18,0x18,0x00,0x00,0x00,0x00,0x00,0x00
    },
    // [5] => '5'
    {
        0x7E,0x7E,0x60,0x60,0x7C,0x7E,0x67,0x03,0x03,0x03,
        0x07,0x7E,0x7C,0x70,0x60,0xC0,0xC0,0xC3,0xC3,0xC3,
        0xC3,0xC3,0xE7,0x7E,0x7C,0x38,0x70,0x60,0x60,0x7E,
        0x7E,0x7E,0x66,0xC3,0xC3,0xE7,0x7E,0x3C,0x00,0x00
    },
    // [6] => '6'
    {
        0x3C,0x7E,0xE7,0xC7,0xC7,0xC7,0x80,0x80,0x9C,0xFE,
        0xFE,0xE7,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xC3,0xE7,
        0x7E,0x3C,0x7C,0xFE,0xC7,0xC3,0xC3,0xE7,0x7E,0x38,
        0x1C,0x0E,0x06,0xC6,0xE6,0x7C,0x38,0x00,0x00,0x00
    },
    // [7] => '7'
    {
        0xFF,0xFF,0x07,0x0E,0x0C,0x1C,0x18,0x38,0x30,0x70,
        0x60,0x60,0xE0,0xC0,0xC0,0xC1,0xC3,0xC6,0xCE,0x1C,
        0x38,0x30,0x70,0x70,0x70,0x70,0x70,0x70,0x70,0x70,
        0x70,0x70,0x30,0x30,0x00,0x00,0x00,0x00,0x00,0x00
    },
    // [8] => '8'
    {
        0x3C,0x7E,0xE7,0xC3,0xC3,0xE7,0x7E,0x3C,0x3C,0x7E,
        0xE7,0xC3,0xC3,0xE7,0x7E,0x3C,0x3C,0x7E,0xE7,0xC3,
        0xC3,0xE7,0x7E,0x3C,0x38,0x6C,0xC6,0xC6,0xC6,0x6C,
        0x38,0x6C,0xC6,0xC6,0xC6,0xFE,0x7C,0x38,0x00,0x00
    },
    // [9] => '9'
    {
        0x3C,0x7E,0xE7,0xC3,0xC3,0xE7,0x7E,0x3C,0x38,0x7C,
        0xE6,0xC6,0xC6,0xC7,0xC7,0xEF,0x7E,0x3C,0x04,0x0C,
        0x18,0x30,0x70,0x60,0xE1,0xC3,0xC7,0xCE,0x7C,0xF8,
        0xF0,0xE0,0xC0,0x80,0x80,0x83,0xC7,0x7E,0x3C,0x00
    },
    // [10] => '.' (小数点)
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x3C,0x7E,0x7E,0x7E,0x7E,0x3C,0x00
    },
    // [11] => '°'
    {
        0x38,0x7C,0xFE,0xC6,0xC6,0xFE,0x7C,0x38,0x10,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    // [12] => '%'
    {
        0xC0,0xC4,0xCC,0xD8,0x70,0x60,0xC0,0x80,0x03,0x06,
        0x0C,0x19,0x33,0x66,0xCC,0x98,0x31,0x63,0x66,0xCC,
        0xD8,0x70,0x60,0xC0,0x80,0x03,0x06,0x0C,0x18,0x31,
        0x63,0x66,0xCC,0xD8,0x70,0x60,0xC0,0x80,0x00,0x00
    },
    // [13] => '-'
    {
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x7E,0x7E,0x7E,0x7E,0x7E,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00
    },
    // [14] => ':'
    {
        0x00,0x18,0x3C,0x3C,0x18,0x18,0x3C,0x3C,0x18,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x18,0x3C,0x3C,0x18,0x18,0x3C,0x3C,0x18,0x00
    },
    // [15] => 'C'
    {
        0x1E,0x3F,0x73,0xE1,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,
        0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xE1,0x73,
        0x3F,0x1E,0x18,0x3C,0x66,0x66,0x66,0x7E,0x7C,0x38,
        0x38,0x7C,0x7E,0x66,0x66,0x66,0x3C,0x18,0x00,0x00
    }
};


/******************************************************************************
 * Local pre-processor symbols/macros ('#define')                             
 ******************************************************************************/

/*****************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

// 将浮点数 data 转为字符串(例如保留三位小数)，并加上后缀 suffix。
// 如果允许负温度，就用 int16_t 来存整数部分，以支持负数。
// decimalPlaces=3 -> 例如 12.345
static void floatToStr(char *buf, size_t bufSize, float data, const char *suffix)
{
    int16_t intPart;
    float fdecimal;
    int16_t decPart;

    // 1) 拿到整数部分（向 0 截断）
    intPart = (int16_t)data;  // 如 data=-12.345 => intPart=-12

    // 2) 计算小数部分(绝对值)，放大 10^3(三位)
    //    若要四位，就乘以10000
    fdecimal = (data - intPart) * 1000.0f;
    if (fdecimal < 0)
    {
        fdecimal = -fdecimal;  // 取正
    }

    // 3) 转成整形
    decPart = (int16_t)fdecimal;  // 不做四舍五入，只截断

    // 4) 用 snprintf 拼接
    //    例如: -12.345°C
    //    注意："%d.%03d" 里没有 %f，避免浮点格式化
    //    %03d => 三位，不足前面补零
    snprintf(buf, bufSize, "%d.%03d%s", intPart, decPart, suffix);
}

// 屏幕数据，每行 19 个字节
static unsigned char screen[HEIGHT][BYTES_PER_ROW];

// 初始化屏幕为白色
void DRAW_initScreen() 
{
    memset(screen, 0xFF, sizeof(screen)); // 设置所有位为 1，即全白
}

// 设置某个像素点 (x, y)，color 为 0 或 1
void DRAW_pixel(uint16_t x, uint16_t y, boolean_t color) 
{
    uint16_t byteIndex = 0;
    uint16_t bitIndex = 0;
    if ((x < WIDTH)  && (y < HEIGHT)) 
	  {
        byteIndex = x / 8;
        bitIndex = 7 - (x % 8);
        if (!color) {
            screen[y][byteIndex] &= ~(1 << bitIndex); // 设置为黑色
        } else {
            screen[y][byteIndex] |= (1 << bitIndex);  // 设置为白色
        }
    }
}

// 绘制水平直线，直接操作数组
void DRAW_hLine(uint16_t x, uint16_t y, uint16_t length, boolean_t color) 
{
    uint16_t startByte = x / 8;
    uint16_t endByte = (x + length - 1) / 8;
    uint16_t startBit = 7 - (x % 8);
    uint16_t endBit = 7 - ((x + length - 1) % 8);
    int i;
    uint8_t mask = 0;
    uint8_t startMask = 0;
    uint8_t endMask = 0;
    if ((y >= HEIGHT) || (x >= WIDTH) || (length <= 0))
		{
        return;
    }
    if (x + length > WIDTH) { // 处理超出右边界
        length = WIDTH - x;
    }

    if (startByte == endByte) {
        // 在同一个字节内
        mask = ((1 << (startBit + 1)) - 1) & ~((1 << endBit) - 1);
        if (!color) 
        {
            screen[y][startByte] &= ~mask; // 设置为黑色
        } 
        else 
        {
            screen[y][startByte] |= mask;  // 设置为白色
        }
    }
     else 
    {
        // 跨多个字节
        startMask = (1 << (startBit + 1)) - 1;
        endMask = ~((1 << endBit) - 1);

        if (!color) {
            screen[y][startByte] &= ~startMask;
            for (i = startByte + 1; i < endByte; i++) {
                screen[y][i] = 0x00;
            }
            screen[y][endByte] &= ~endMask;
        } else {
            screen[y][startByte] |= startMask;
            for (i = startByte + 1; i < endByte; i++) {
                screen[y][i] = 0xFF;
            }
            screen[y][endByte] |= endMask;
        }
    }
}

// 绘制垂直直线，直接操作数组
void DRAW_vLine(uint16_t x, uint16_t y, uint16_t length, boolean_t color) 
{
    uint16_t byteIndex = x / 8;
    uint16_t bitIndex = 7 - (x % 8);
    unsigned char mask = (1 << bitIndex);
    int i;
    if (x >= WIDTH || y >= HEIGHT || length <= 0) 
		{
        return;
    }
    if (y + length > HEIGHT) { // 处理超出下边界
        length = HEIGHT - y;
    }

    if (!color) {
        for (i = 0; i < length; i++) {
            screen[y + i][byteIndex] &= ~mask; // 设置为黑色
        }
    } else {
        for (i = 0; i < length; i++) {
            screen[y + i][byteIndex] |= mask;  // 设置为白色
        }
    }
}

// 绘制矩形，filled 为 1 表示实心，0 表示空心
void DRAW_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, boolean_t color, boolean_t filled) 
{
    uint16_t i;
    if (filled) 
    {
        for (i = 0; i < height; i++) 
        {
            DRAW_hLine(x, y + i, width, color);
        }
    } 
    else 
    {
        DRAW_hLine(x, y, width, color);                 // 上边
        DRAW_hLine(x, y + height - 1, width, color);    // 下边
        DRAW_vLine(x, y, height, color);               // 左边
        DRAW_vLine(x + width - 1, y, height, color);   // 右边
    }
}

void DRAW_outputScreen(void)
{
    EPD_display(screen);
}

void DRAW_char(int x, int y, char c, int fontSize, int color)
{
    int row, col, dx, dy;
    int byteIndex, bitIndex, screenRow, screenCol;
    const unsigned char *glyph;

    // 判断字符范围并选择字体数据
    if (c >= '0' && c <= '9')
    {
        c -= '0';
    }
    else if (c >= 'A' && c <= 'Z')
    {
        c -= 'A' - 10;
    }
    else if (c >= 'a' && c <= 'z')
    {
        c -= 'a' - 36;
    }
    else
    {
        return; // 不支持的字符
    }

    glyph = FONT_5X7[(int)c];

    if (fontSize == 1)
    {
        for (row = 0; row < 7; row++) // 遍历字体的每一行
        {
            screenRow = y + row; // 屏幕上的行
            if (screenRow < 0 || screenRow >= HEIGHT)
            {
                continue; // 超出屏幕范围，跳过
            }

            for (col = 0; col < 5; col++) // 遍历字体的每一列
            {
                screenCol = x + col; // 屏幕上的列
                if (screenCol < 0 || screenCol >= WIDTH)
                {
                    continue; // 超出屏幕范围，跳过
                }

                byteIndex = screenCol / 8; // 字节索引
                bitIndex = 7 - (screenCol % 8); // 位索引（高位在左）

                // 按行扫描字体数据，决定是否设置像素
                if (glyph[col] & (1 << row)) // 每列的第 row 位
                {
                    if (color == 1)
                    {
                        screen[screenRow][byteIndex] |= (1 << bitIndex); // 设为白色
                    }
                    else
                    {
                        screen[screenRow][byteIndex] &= ~(1 << bitIndex); // 设为黑色
                    }
                }
            }
        }
    }
    else // 字体放大处理
    {
        for (row = 0; row < 7; row++)
        {
            for (col = 0; col < 5; col++)
            {
                if (glyph[col] & (1 << row)) // 每列的第 row 位
                {
                    for (dy = 0; dy < fontSize; dy++)
                    {
                        screenRow = y + row * fontSize + dy;
                        if (screenRow < 0 || screenRow >= HEIGHT)
                        {
                            continue; // 超出屏幕范围，跳过
                        }

                        for (dx = 0; dx < fontSize; dx++)
                        {
                            screenCol = x + col * fontSize + dx;
                            if (screenCol < 0 || screenCol >= WIDTH)
                            {
                                continue; // 超出屏幕范围，跳过
                            }

                            byteIndex = screenCol / 8; // 字节索引
                            bitIndex = 7 - (screenCol % 8); // 位索引（高位在左）

                            if (color == 1)
                            {
                                screen[screenRow][byteIndex] |= (1 << bitIndex); // 设为白色
                            }
                            else
                            {
                                screen[screenRow][byteIndex] &= ~(1 << bitIndex); // 设为黑色
                            }
                        }
                    }
                }
            }
        }
    }
}

void DRAW_string(int x, int y, const char *str, int fontSize, int color)
{
    int cursorX;
    cursorX = x;

    while (*str)
    {
        DRAW_char(cursorX, y, *str, fontSize, color);
        cursorX += (5 * fontSize + 1); // 每个字符宽度为 5，加一个像素间隔
        str++;
    }
}

void DRAW_rotatedChar(int x, int y, char c, int fontSize, int color)
{
    int row, col, dx, dy;
    int originalRow, originalCol;
    int byteIndex, bitIndex, screenRow, screenCol;
    const unsigned char *glyph;

    // 判断字符范围并选择字体数据
    if (c >= '0' && c <= '9')
    {
        c -= '0';
    }
    else if (c >= 'A' && c <= 'Z')
    {
        c -= 'A' - 10;
    }
    else if (c >= 'a' && c <= 'z')
    {
        c -= 'a' - 36;
    }
    else
    {
        return; // 不支持的字符
    }

    glyph = FONT_5X7[(int)c];

    if (fontSize == 1)
    {
        for (row = 0; row < 7; row++) // 遍历字体的每一行
        {
            for (col = 0; col < 5; col++) // 遍历字体的每一列
            {
                if (glyph[col] & (1 << row)) // 每列的第 row 位
                {
                    // 逆时针旋转坐标
                    originalRow = y + row;
                    originalCol = x + col;
                    screenRow = WIDTH - 1 - originalCol; // 新行
                    screenCol = originalRow;            // 新列

                    if (screenRow < 0 || screenRow >= HEIGHT || screenCol < 0 || screenCol >= WIDTH)
                    {
                        continue; // 超出屏幕范围，跳过
                    }

                    byteIndex = screenCol / 8;          // 字节索引
                    bitIndex = 7 - (screenCol % 8);     // 位索引

                    if (color == 1)
                    {
                        screen[screenRow][byteIndex] |= (1 << bitIndex); // 设为白色
                    }
                    else
                    {
                        screen[screenRow][byteIndex] &= ~(1 << bitIndex); // 设为黑色
                    }
                }
            }
        }
    }
    else // 字体放大处理
    {
        for (row = 0; row < 7; row++)
        {
            for (col = 0; col < 5; col++)
            {
                if (glyph[col] & (1 << row)) // 每列的第 row 位
                {
                    for (dy = 0; dy < fontSize; dy++)
                    {
                        for (dx = 0; dx < fontSize; dx++)
                        {
                            // 逆时针旋转坐标
                            originalRow = y + row * fontSize + dy;
                            originalCol = x + col * fontSize + dx;
                            screenRow = WIDTH - 1 - originalCol; // 新行
                            screenCol = originalRow;            // 新列

                            if (screenRow < 0 || screenRow >= HEIGHT || screenCol < 0 || screenCol >= WIDTH)
                            {
                                continue; // 超出屏幕范围，跳过
                            }

                            byteIndex = screenCol / 8;          // 字节索引
                            bitIndex = 7 - (screenCol % 8);     // 位索引

                            if (color == 1)
                            {
                                screen[screenRow][byteIndex] |= (1 << bitIndex); // 设为白色
                            }
                            else
                            {
                                screen[screenRow][byteIndex] &= ~(1 << bitIndex); // 设为黑色
                            }
                        }
                    }
                }
            }
        }
    }
}

void DRAW_rotatedString(int x, int y, const char *str, int fontSize, int color)
{
    int offset = 0; // 字符间的水平偏移

    while (*str)
    {
        DRAW_rotatedChar(x + offset, y, *str, fontSize, color);
        offset += 6 * fontSize; // 每个字符宽度（5 像素 + 1 像素间距）
        str++;
    }
}

// c: 字符 ('0'..'9', '.', '°', '%', '-', ':', 'C')
// color=1 => 点亮(白), 0 => 熄灭(黑) (视硬件而定)
void DRAW_Char40_Rot(int x, int y, char c, int color)
{
    int index;
    int row;
    int bit;
    unsigned char lineByte;
    int originalRow;
    int originalCol;
    int screenRow;
    int screenCol;
    int byteIndex;
    int bitIndex;

    index = -1;

    if (c >= '0' && c <= '9')
    {
        // '0'..'9' => 下标0..9
        index = c - '0';
    }
    else if (c == '.')
    {
        index = 10;
    }
    else if (c == '°')
    {
        index = 11;
    }
    else if (c == '%')
    {
        index = 12;
    }
    else if (c == '-')
    {
        index = 13;
    }
    else if (c == ':')
    {
        index = 14;
    }
    else if (c == 'C')
    {
        index = 15;
    }
    else
    {
        // 不支持的字符
        return;
    }

    for (row = 0; row < 40; row++)
    {
        lineByte = FONT_40PX[index][row];

        for (bit = 7; bit >= 0; bit--)
        {
            if ((lineByte >> bit) & 0x01)
            {
                originalRow = y + row;
                originalCol = x + (7 - bit);

                // 逆时针90度
                screenRow = (WIDTH - 1) - originalCol;
                screenCol = originalRow;

                if ((screenRow >= 0) && (screenRow < HEIGHT) &&
                    (screenCol >= 0) && (screenCol < WIDTH))
                {
                    byteIndex = screenCol / 8;
                    bitIndex = 7 - (screenCol % 8);

                    if (color == 1)
                    {
                        // 点亮(假设1=白色)
                        screen[screenRow][byteIndex] |= (1 << bitIndex);
                    }
                    else
                    {
                        // 熄灭
                        screen[screenRow][byteIndex] &= ~(1 << bitIndex);
                    }
                }
            }
        }
    }
}

void DRAW_String40_Rot(int x, int y, const char *str, int color)
{
    int curX, i;
    curX = x;
    i = 0;

    // 假设每个字符的宽度 = 8 像素
    while (str[i] != '\0')
    {
        DRAW_Char40_Rot(curX, y, str[i], color);
        curX += 8; // 下一个字符向右偏移8像素
        i++;
    }
}

void DRAW_DisplayTempHumiRot(float temperature, float humidity)
{
    char tempBuf[32];
    char humBuf[32];
    int baseX = 10;
    int baseY = 20;


    // 1) 生成温度字符串 => 类似  -12.345°C
    floatToStr(tempBuf, sizeof(tempBuf), temperature, "°C");

    // 2) 生成湿度字符串 => 类似  78.123%
    //    若你要湿度始终为正，可以在 floatToStr 里强制 data= fabsf(data) 
    //    或者直接写个2号函数(去掉负号处理)，这里演示最简便
    floatToStr(humBuf, sizeof(humBuf), humidity, "%");

    // 3) 在屏幕上显示
    //    第1行：温度
    //    第2行：湿度 (y坐标+50)
        DRAW_rotatedString(baseX, baseY, tempBuf,4, 0);

        baseY += 70;  // 分行(40像素字体+点间距)
        DRAW_rotatedString(baseX, baseY, humBuf,4, 0);
}

// void DRAW_rotateScreen90()
// {
//     unsigned char tempScreen[HEIGHT][WIDTH / 8]; // 临时数组保存旋转后的数据
//     int row, col, byteIndex, bitIndex, newRow, newCol, newByteIndex, newBitIndex;

//     // 初始化临时数组
//     for (row = 0; row < HEIGHT; row++)
//     {
//         for (col = 0; col < WIDTH / 8; col++)
//         {
//             tempScreen[row][col] = 0; // 清空临时屏幕
//         }
//     }

//     // 遍历原屏幕数组
//     for (row = 0; row < HEIGHT; row++)
//     {
//         for (col = 0; col < WIDTH; col++)
//         {
//             byteIndex = col / 8;          // 原列所在的字节索引
//             bitIndex = 7 - (col % 8);     // 原列所在的位索引

//             // 检查原位置是否为1（白色像素）
//             if (screen[row][byteIndex] & (1 << bitIndex))
//             {
//                 // 新坐标计算（逆时针旋转90°）
//                 newRow = WIDTH - 1 - col; // 旋转后的行（原列翻转）
//                 newCol = row;             // 旋转后的列（原行）

//                 // 计算新位置的字节索引和位索引
//                 newByteIndex = newCol / 8; // 新列所在的字节索引
//                 newBitIndex = 7 - (newCol % 8); // 新列所在的位索引

//                 // 更新临时数组
//                 tempScreen[newRow][newByteIndex] |= (1 << newBitIndex);
//             }
//         }
//     }

//     // 将临时数组内容拷贝回原屏幕数组
//     for (row = 0; row < HEIGHT; row++)
//     {
//         for (col = 0; col < WIDTH / 8; col++)
//         {
//             screen[row][col] = tempScreen[row][col];
//         }
//     }
// }

// 返回屏幕数据指针
// unsigned char (*DRAW_outputScreen())[BYTES_PER_ROW] 
// {
//     return screen;
// }
// // 示例：绘制内容并输出
// int main() {
//     initScreen();
//     drawRectangle(10, 10, 50, 30, 1, 0); // 绘制黑色空心矩形
//     DRAW_hLine(0, 75, 152, 1);           // 绘制黑色水平线
//     DRAW_vLine(75, 0, 152, 1);           // 绘制黑色垂直线
//     outputScreen();
//     return 0;
// }

/******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/


