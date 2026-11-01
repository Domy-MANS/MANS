#include "pid.h"

// PID控制器初始化
void PID_Init(PID_Controller *pid, float kp, float ki, float kd, float integral_limit, float output_limit)
{
    pid->Kp = kp;
    pid->Ki = ki;
    pid->Kd = kd;
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
    pid->integral_limit = integral_limit;
    pid->output_limit = output_limit;
}

// PID更新计算
float PID_Update(PID_Controller *pid, float setpoint, float measured_value)
{
    // 计算误差
    float error = setpoint - measured_value;
    
    // 积分项（带限幅）
    pid->integral += error;
    if (pid->integral > pid->integral_limit) pid->integral = pid->integral_limit;
    if (pid->integral < -pid->integral_limit) pid->integral = -pid->integral_limit;
    
    // 微分项
    float derivative = error - pid->last_error;
    
    // 计算输出
    pid->output = pid->Kp * error + pid->Ki * pid->integral + pid->Kd * derivative;
    
    // 输出限幅
    if (pid->output > pid->output_limit) pid->output = pid->output_limit;
    if (pid->output < -pid->output_limit) pid->output = -pid->output_limit;
    
    // 保存上次误差
    pid->last_error = error;
    
    return pid->output;
}

// 重置PID控制器
void PID_Reset(PID_Controller *pid)
{
    pid->last_error = 0.0f;
    pid->integral = 0.0f;
    pid->output = 0.0f;
}

