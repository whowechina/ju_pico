#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "pico/time.h"

#include "cli.h"
#include "matrix.h"
#include "hub75.h"

static void dma_complete(uint16_t row, uint16_t bit)
{
    if (row == 0 && bit == 0) {
        cli_fps_count(2);
    }
}

void matrix_init()
{
    hub75_init(true, false);
    hub75_start(dma_complete);
}

void draw_button(uint8_t x, uint8_t y, uint32_t border, uint32_t solid)
{
    int col = x * 17;
    int row = y * 17;
    for (int i = 0; i < 13; i++) {
        hub75_pixel(col + i, row, border);
        hub75_pixel(col + i, row + 12, border);
        hub75_pixel(col, row + i, border);
        hub75_pixel(col + 12, row + i, border);
    }

    for (int i = 1; i < 12; i++) {
        for (int j = 1; j < 12; j++) {
            hub75_pixel(col + i, row + j, solid);
        }
    }
}

static void draw_door(uint8_t x, uint8_t y, uint32_t progress, bool arrow)
{
    const uint32_t solid = hub75_rgb(0, 140, 140);
    int col = x * 17;
    int row = y * 17;
    for (int i = 0; i < 13; i++) {
        int len = arrow ? abs(i - 6) + 1 : 0;
        int bottom = len + 11;

        int prog = progress * 255 * 12 / 1000;
 
        int split = prog / 255;
        int extra = prog % 255;
        for (int j = 0; j < 13; j++) {
            uint32_t color = 0;
            if (j < len) {
                color = solid;
            } else if (j > bottom - split) {
                color = solid;
            }
            hub75_pixel(col + i, row + j, color);
        }
        if (bottom - split < 13) {
            hub75_blend(col + i, row + bottom - split, hub75_alpha(extra, solid));
        }
    }
}

static void draw_sector_fast(uint8_t x, uint8_t y, uint32_t progress)
{
    const uint32_t solid = hub75_rgb(255, 100, 50);
    int col = x * 17;
    int row = y * 17;
    
    int center = 6;
    
    // 进度转换为角度 (0-360度，用整数避免浮点运算)
    int angle_max = (progress * 360) / 1000;
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            int dx = i - center;
            int dy = j - center;
            
            // 计算角度(用整数近似，从12点方向开始顺时针)
            int angle;
            if (dx == 0 && dy < 0) angle = 0;     // 正北
            else if (dx > 0 && dy < 0) angle = 45;   // 东北
            else if (dx > 0 && dy == 0) angle = 90;  // 正东
            else if (dx > 0 && dy > 0) angle = 135;  // 东南
            else if (dx == 0 && dy > 0) angle = 180; // 正南
            else if (dx < 0 && dy > 0) angle = 225;  // 西南
            else if (dx < 0 && dy == 0) angle = 270; // 正西
            else if (dx < 0 && dy < 0) angle = 315;  // 西北
            else angle = 0; // 中心点
            
            // 更精细的角度计算（可选）
            if (dx != 0 || dy != 0) {
                // 简化的角度计算，用查表法或线性插值
                if (abs(dx) > abs(dy)) {
                    // 主要在东西方向
                    if (dx > 0) {
                        angle = 90 + (dy * 45) / abs(dx);
                    } else {
                        angle = 270 - (dy * 45) / abs(dx);
                    }
                } else {
                    // 主要在南北方向
                    if (dy < 0) {
                        angle = 0 + (dx * 45) / abs(dy);
                    } else {
                        angle = 180 - (dx * 45) / abs(dy);
                    }
                }
                if (angle < 0) angle += 360;
            }
            
            bool in_sector = (angle <= angle_max) || (angle_max >= 360);
            
            // 边缘抗锯齿
            if (!in_sector && angle_max < 360) {
                int angle_diff = angle - angle_max;
                if (angle_diff < 10 && angle_diff > 0) {  // 10度的渐变区域
                    uint8_t alpha = 255 * (10 - angle_diff) / 10;
                    hub75_blend(col + i, row + j, hub75_alpha(alpha, solid));
                }
            } else if (in_sector) {
                hub75_pixel(col + i, row + j, solid);
            }
        }
    }
}

static void draw_closing_door_smooth(uint8_t x, uint8_t y, uint32_t progress, bool invert)
{
    const uint32_t solid = hub75_rgb(100, 255, 100);
    int col = x * 17;
    int row = y * 17;
    
    // 精确的关门进度 (0-6.5的浮点数效果，用整数模拟)
    int close_distance_x10 = (progress * 65) / 1000;  // 0-65 (代表0-6.5)
    int full_close = close_distance_x10 / 10;
    int partial_close = close_distance_x10 % 10;
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            // 计算到中心或边缘的距离
            int distance = invert ? 
                ((j <= 6) ? j : (12 - j)) :  // 距离边缘
                abs(j - 6);                  // 距离中心
            
            // 直接计算并绘制
            if (distance < full_close) {
                hub75_pixel(col + i, row + j, solid);
            } else if (distance == full_close && full_close <= 6) {
                uint8_t alpha = (partial_close * 255) / 10;
                hub75_blend(col + i, row + j, hub75_alpha(alpha, solid));
            }
        }
    }
}

// 简化的整数三角函数表 (256个值，0-360度)
static const int sin_table[] = {
    0, 4, 9, 13, 18, 22, 27, 31, 36, 40, 44, 49, 53, 58, 62, 66, 71, 75, 79, 83,
    88, 92, 96, 100, 104, 108, 112, 116, 120, 124, 128, 131, 135, 139, 143, 146, 150, 153, 157, 160,
    164, 167, 171, 174, 177, 181, 184, 187, 190, 193, 196, 199, 202, 205, 208, 211, 213, 216, 219, 221,
    224, 226, 229, 231, 233, 235, 238, 240, 242, 244, 246, 247, 249, 251, 252, 254, 255, 257, 258, 259,
    260, 262, 263, 264, 265, 266, 267, 267, 268, 269, 269, 270
};

// 获取sin值 (输入0-360度，输出-270到270的定点数)
static int get_sin(int deg) {
    deg = deg % 360;
    if (deg < 0) deg += 360;
    
    if (deg <= 90) return sin_table[deg];
    else if (deg <= 180) return sin_table[180 - deg];
    else if (deg <= 270) return -sin_table[deg - 180];
    else return -sin_table[360 - deg];
}

// 获取cos值 (cos(x) = sin(x + 90))
static int get_cos(int deg) {
    return get_sin(deg + 90);
}

// 旋转函数：将canvas中(x,y)位置的13x13格子按指定角度旋转
void rotate_13x13(uint32_t canvas[][64], int x, int y, int deg) {
    // 临时缓冲区存储旋转后的结果
    uint32_t temp[13][13];
    
    // 清空临时缓冲区
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            temp[i][j] = 0;
        }
    }
    
    // 获取旋转矩阵的整数值 (放大270倍)
    int cos_deg = get_cos(deg);
    int sin_deg = get_sin(deg);
    
    // 中心点 (6, 6)
    int center = 6;
    
    // 对每个输出像素进行反向变换采样
    for (int out_i = 0; out_i < 13; out_i++) {
        for (int out_j = 0; out_j < 13; out_j++) {
            // 输出像素相对于中心的坐标
            int out_x = (out_i - center) * 270;
            int out_y = (out_j - center) * 270;
            
            // 反向旋转得到源坐标 (使用-deg)
            int src_x = (out_x * cos_deg + out_y * sin_deg) / 270 + center * 270;
            int src_y = (-out_x * sin_deg + out_y * cos_deg) / 270 + center * 270;
            
            // 转换为像素坐标 (270倍放大)
            int src_i = src_x / 270;
            int src_j = src_y / 270;
            
            // 检查边界
            if (src_i >= 0 && src_i < 12 && src_j >= 0 && src_j < 12) {
                // 计算小数部分用于双线性插值
                int frac_x = src_x % 270;
                int frac_y = src_y % 270;
                
                // 获取四个相邻像素的颜色值 (从canvas中读取)
                uint32_t c00 = canvas[x + src_i][y + src_j];
                uint32_t c10 = canvas[x + src_i + 1][y + src_j];
                uint32_t c01 = canvas[x + src_i][y + src_j + 1];
                uint32_t c11 = canvas[x + src_i + 1][y + src_j + 1];
                
                // 双线性插值计算最终颜色
                // 简化版本：如果有任何相邻像素有颜色，就使用最亮的那个
                uint32_t result_color = 0;
                if (c00 || c10 || c01 || c11) {
                    // 选择最亮的颜色作为结果
                    result_color = c00;
                    if ((c10 & 0x3FF3FF3FF) > (result_color & 0x3FF3FF3FF)) result_color = c10;
                    if ((c01 & 0x3FF3FF3FF) > (result_color & 0x3FF3FF3FF)) result_color = c01;
                    if ((c11 & 0x3FF3FF3FF) > (result_color & 0x3FF3FF3FF)) result_color = c11;
                    
                    // 根据距离调整亮度来实现抗锯齿
                    int weight = 270 - ((frac_x + frac_y) / 2);
                    if (weight < 270) {
                        uint8_t alpha = (weight * 255) / 270;
                        result_color = hub75_alpha(alpha, result_color);
                    }
                }
                
                temp[out_i][out_j] = result_color;
            }
        }
    }
    
    // 将旋转结果写回canvas
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            canvas[x + i][y + j] = temp[i][j];
        }
    }
}

static void rotate_90(int x, int y, int times)
{
    times = times % 4;
    
    if (times == 0) {
        return;
    }

    int col = x * 17;
    int row = y * 17;

    uint32_t temp[13][13];
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            temp[i][j] = canvas[col + i][row + j];
        }
    }
    
    for (int i = 0; i < 13; i++) {
        for (int j = 0; j < 13; j++) {
            int src_i;
            int src_j;

            if (times == 1) {
                src_i = 12 - j;
                src_j = i;
            } else if (times == 2) {
                src_i = 12 - i;
                src_j = 12 - j;
            } else {
                src_i = j;
                src_j = 12 - i;
            }

            canvas[col + i][row + j] = temp[src_i][src_j];
        }
    }
}

static void draw()
{
    int phase = (time_us_64() / 1000) % 1001;
    for (int i = 0; i < 16; i++) {
        int col = i % 4;
        int row = i / 4;
        int progress = (phase + (i * 1000 / 15)) % 1100;
        if (progress > 1000) {
            progress = 1000;
        }
        int choice = i % 6;
        if (choice == 0) {
            draw_sector_fast(col, row, progress);
        } else if (choice == 1) {
            draw_door(col, row, progress, false);
        } else if (choice == 2) {
            draw_door(col, row, progress, true);
        } else if (choice == 3) {
            // 正常关门效果 - 上下
            draw_closing_door_smooth(col, row, progress, false);
        } else if (choice == 4) {
            // 反转关门效果
            draw_closing_door_smooth(col, row, progress, true);
        } else {
            // 扇形展开（重复使用）
            draw_sector_fast(col, row, progress);
        }

        rotate_90(col, row, col + row);
    }
}

void overlay()
{
    int off = abs((time_us_32() / 100000 % 80) - 40);
    uint32_t color = 0x20000000 | hub75_hsv2rgb(time_us_32() / 20000, 200, 255);
    for (int i = 0; i < 23; i++) {
        for (int j = 0; j < 23; j++) {
            hub75_blend(off + i, off + j, color);
        }
    }
}

void matrix_update()
{
    hub75_clear();
    draw();
    overlay();
    hub75_update();
}
