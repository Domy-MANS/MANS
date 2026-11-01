#include "speed_mapper.h"
#include <math.h>

// 速度映射全局变量
Speed_Mapper_t speed_mapper;

// 速度映射器初始化
void SpeedMapper_Init(Speed_Mapper_t *mapper, float min_speed, float max_speed, 
                     float min_pwm, float max_pwm, float dead_zone)
{
    mapper->min_speed = min_speed;
    mapper->max_speed = max_speed;
    mapper->min_pwm = min_pwm;
    mapper->max_pwm = max_pwm;
    mapper->dead_zone = dead_zone;
}

// 将速度转换为PWM占空比
uint16_t SpeedToPWM(Speed_Mapper_t *mapper, float speed)
{
    // 限制速度范围
    if (speed > mapper->max_speed) speed = mapper->max_speed;
    if (speed < -mapper->max_speed) speed = -mapper->max_speed;
    
    // 计算归一化速度 (-1 到 +1)
    float normalized_speed = speed / mapper->max_speed;
    
    // 应用死区补偿
    if (fabsf(normalized_speed) < mapper->dead_zone) {
        normalized_speed = 0.0f;
    }
    
    // 线性映射到PWM范围
    float pwm_value;
    if (normalized_speed >= 0) {
        pwm_value = mapper->min_pwm + normalized_speed * (mapper->max_pwm - mapper->min_pwm);
    } else {
        pwm_value = mapper->min_pwm + normalized_speed * (mapper->min_pwm - 0);
    }
    
    // 确保PWM在有效范围内
    if (pwm_value > mapper->max_pwm) pwm_value = mapper->max_pwm;
    if (pwm_value < 0) pwm_value = 0;
    
    return (uint16_t)pwm_value;
}

// 根据位置误差计算目标速度
float GetSpeedFromPositionPID(float position_error, float max_speed)
{
    // 使用非线性函数将位置误差映射到目标速度
    // 当误差大时，速度大；误差小时，速度小
    
    // 限制位置误差范围 (-180° 到 +180°)
    if (position_error > 180.0f) position_error = 180.0f;
    if (position_error < -180.0f) position_error = -180.0f;
    
    // 使用反正切函数创建平滑的速度曲线
    float normalized_error = position_error / 180.0f;  // 归一化到 -1 到 +1
    float target_speed = atanf(normalized_error * 3.0f) * (2.0f / 3.14159f) * max_speed;
    
    return target_speed;
}

