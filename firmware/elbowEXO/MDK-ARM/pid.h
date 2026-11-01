#ifndef __PID_H
#define __PID_H

#include "main.h"

// PID控制器结构体定义
typedef struct {
    float Kp, Ki, Kd;      // PID参数
    float last_error;       // 上次误差
    float integral;         // 积分项
    float output;           // 当前输出值
    float integral_limit;   // 积分限幅
    float output_limit;     // 输出限幅
} PID_Controller;

// 函数声明
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, float integral_limit, float output_limit);
float PID_Update(PID_Controller *pid, float setpoint, float measured_value);
void PID_Reset(PID_Controller *pid);

#endif

