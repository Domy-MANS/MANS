#include "driver.h"
#include <stdio.h>

// 电机全局变量
Motor_TypeDef motor;
extern TIM_HandleTypeDef htim1;

// 电机初始化
void Motor_Init(TIM_HandleTypeDef *htim)
{
    // 保存定时器句柄
    htim1 = *htim;
    
    // 初始化电机状态
    motor.state = MOTOR_STOPPED;
    motor.direction = MOTOR_DIR_CW;
    motor.pwm_duty = 0;
    motor.fault_flag = 0;
    motor.current_limit = 0;
    
    // 唤醒驱动器
    Motor_Wakeup();
    
    // 设置默认方向
    Motor_SetDirection(MOTOR_DIR_CW);
    
    // 启动PWM
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);
    
    // 初始停止电机
    Motor_Stop();
    
}

// 设置电机方向
void Motor_SetDirection(uint8_t dir)
{
    motor.direction = dir;
    
    if (dir == MOTOR_DIR_CW) {
        HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN, GPIO_PIN_RESET);
    } else {
        HAL_GPIO_WritePin(MOTOR_PH_PORT, MOTOR_PH_PIN, GPIO_PIN_SET);
    }
}

// 设置电机速度 (duty: 0-1000 对应 0-100%)
void Motor_SetSpeed(uint16_t duty)
{
    // 限制PWM占空比范围
    if (duty > 1000) duty = 1000;
    
    motor.pwm_duty = duty;
    
    // 设置PWM占空比 (TIM1 ARR = 999, 所以占空比 = duty)
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty);
    
    // 更新状态
    if (duty == 0) {
        motor.state = MOTOR_STOPPED;
    } else {
        motor.state = MOTOR_RUNNING;
    }
}

// 停止电机
void Motor_Stop(void)
{
    Motor_SetSpeed(0);
}

// 启动电机（确保唤醒状态）
void Motor_Start(void)
{
    Motor_Wakeup();
    motor.state = MOTOR_RUNNING;
}

// 进入睡眠模式
void Motor_Sleep(void)
{
    HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_RESET);
    motor.state = MOTOR_STOPPED;
}

// 唤醒驱动器
void Motor_Wakeup(void)
{
    HAL_GPIO_WritePin(MOTOR_NSLEEP_PORT, MOTOR_NSLEEP_PIN, GPIO_PIN_SET);
    HAL_Delay(1); // 等待驱动器稳定
}

// 检查故障状态
uint8_t Motor_CheckFault(void)
{
    // 读取NFAULT引脚状态（低电平表示故障）
    motor.fault_flag = (HAL_GPIO_ReadPin(MOTOR_NFAULT_PORT, MOTOR_NFAULT_PIN) == GPIO_PIN_RESET);
    return motor.fault_flag;
}

// 检查电流限制状态
uint8_t Motor_CheckCurrentLimit(void)
{
    // 读取SNSOUT引脚状态（低电平表示电流限制）
    motor.current_limit = (HAL_GPIO_ReadPin(MOTOR_SNSOUT_PORT, MOTOR_SNSOUT_PIN) == GPIO_PIN_RESET);
    return motor.current_limit;
}

