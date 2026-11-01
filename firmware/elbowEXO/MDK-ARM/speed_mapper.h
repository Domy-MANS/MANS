#ifndef __SPEED_MAPPER_H
#define __SPEED_MAPPER_H

#include "main.h"

// 速度映射结构体
typedef struct {
    float min_speed;        // 最小速度 
    float max_speed;        // 最大速度
    float min_pwm;          // 最小PWM占空比
    float max_pwm;          // 最大PWM占空比
    float dead_zone;        // 死区补偿
} Speed_Mapper_t;

// 函数声明
void SpeedMapper_Init(Speed_Mapper_t *mapper, float min_speed, float max_speed, 
                     float min_pwm, float max_pwm, float dead_zone);
uint16_t SpeedToPWM(Speed_Mapper_t *mapper, float speed);
float GetSpeedFromPositionPID(float position_error, float max_speed);

extern Speed_Mapper_t speed_mapper;

#endif

