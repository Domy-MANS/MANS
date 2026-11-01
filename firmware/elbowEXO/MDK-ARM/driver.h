#ifndef __DRIVER_H
#define __DRIVER_H

#include "stm32f1xx_hal.h"
#include "main.h"

// 电机驱动引脚定义
#define MOTOR_PH_PIN        GPIO_PIN_13
#define MOTOR_PH_PORT       GPIOB

#define MOTOR_NSLEEP_PIN    GPIO_PIN_2
#define MOTOR_NSLEEP_PORT   GPIOB

#define MOTOR_NFAULT_PIN    GPIO_PIN_0
#define MOTOR_NFAULT_PORT   GPIOB

#define MOTOR_SNSOUT_PIN    GPIO_PIN_1
#define MOTOR_SNSOUT_PORT   GPIOB

// 电机方向定义
#define MOTOR_DIR_CW        0   // 顺时针 (PH=0)
#define MOTOR_DIR_CCW       1   // 逆时针 (PH=1)

// 电机状态定义
typedef enum {
    MOTOR_STOPPED = 0,
    MOTOR_RUNNING,
    MOTOR_FAULT
} Motor_State_t;

// 电机控制结构体
typedef struct {
    Motor_State_t state;
    uint8_t direction;
    uint16_t pwm_duty;      // 0-1000 对应 0-100%
    uint8_t fault_flag;
    uint8_t current_limit;
} Motor_TypeDef;

// 函数声明
void Motor_Init(TIM_HandleTypeDef *htim);
void Motor_SetDirection(uint8_t dir);
void Motor_SetSpeed(uint16_t duty);
void Motor_Stop(void);
void Motor_Start(void);
void Motor_Sleep(void);
void Motor_Wakeup(void);
uint8_t Motor_CheckFault(void);
uint8_t Motor_CheckCurrentLimit(void);

extern Motor_TypeDef motor;

#endif
