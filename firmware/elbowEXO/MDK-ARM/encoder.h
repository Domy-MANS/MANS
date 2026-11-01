#ifndef __ENCODER_H
#define __ENCODER_H

#include "stm32f1xx_hal.h"
#include "main.h"

// 引脚定义
#define ENCODER_CS_PIN          GPIO_PIN_6
#define ENCODER_CS_PORT         GPIOB

#define ENCODER_RESOLUTION      16384.0f    // 14位分辨率
#define ENCODER_GEAR_RATIO      36.0f       // 减速比

// 寄存器地址
#define ENCODER_ANGLE_REG       0x3FFF      // 角度读取寄存器
#define ENCODER_ERROR_REG       0x0001      // 错误状态寄存器

// 编码器数据结构
typedef struct {
    float raw_angle;        // 原始角度(度)
    float filtered_angle;   // 滤波后角度(度)  
    float velocity;         // 速度(度/秒)
    uint16_t raw_data;      // 原始数据
    uint32_t last_time;     // 上次采样时间
    int32_t total_ticks;    // 总脉冲数
    float last_angle;       // 上次角度值（用于速度计算）
} Encoder_TypeDef;

// 函数声明
void Encoder_Init(SPI_HandleTypeDef *hspi);
float Encoder_ReadAngle(void);
float Encoder_ReadVelocity(void);
void Encoder_UpdateVelocity(void);  
uint16_t Encoder_ReadRawData(void);
uint8_t Encoder_CheckError(void);

extern Encoder_TypeDef encoder;

#endif

