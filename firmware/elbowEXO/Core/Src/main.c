/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
// ����ͷ�ļ�
#include "encoder.h"
#include "driver.h" 
#include "pid.h"
#include "speed_mapper.h"
#include <stdio.h>
#include <math.h>
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */
// 全局变量
PID_Controller position_pid;    // 位置环PID
PID_Controller speed_pid;       // 速度环PID
float target_angle = 90.0f;     // 目标角度
float target_speed = 0.0f;      // 目标速度


/////////////////////////////轨迹参数调整处//////////////////////////////////////////////
// 正弦轨迹参数//参数设置注意：1.正弦幅度<正弦偏移;2.正弦偏移-正弦幅度的值差别不能太大，不然会从初始0角度突然剧烈跳变
float trajectory_t = 0.0f;      // 轨迹时间变量
#define TRAJECTORY_AMPLITUDE 44.0f    // 正弦幅度 44度
#define TRAJECTORY_OFFSET 45.0f       // 正弦偏移 45度
#define TRAJECTORY_FREQUENCY 0.1f     // 正弦频率 (Hz)
//////最终表达式  期望角度曲线=-44cos(0.1t + 0)+45
///////////////////////////////////////////////////////////////////////////////////////////
	float Avr[10] = {0.0f};
	float sum = 0.0f;	
	float a = 0.0f;



// 控制参数
#define MAX_MOTOR_SPEED     300.0f  // 最大电机速度 
#define CONTROL_FREQ        100     // 100Hz控制频率
uint32_t control_counter = 0;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
// 双环控制函数声明
void PositionSpeedDualLoopControl(void);
void UpdateTrajectory(void);  //更新轨迹
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
// 重定向printf到串口
int fputc(int ch, FILE *f)
{
  HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 0xffff);
  return ch;
}

// 简单的电机测试函数
void Motor_SimpleTest(void)
{   
	
	    // 1. 更新速度测量
    Encoder_UpdateVelocity();
   
    // 2. 读取当前位置和当前速度
    float angle0 = Encoder_ReadAngle();
	printf("Current_Angle: %f\r\n",angle0);

    // 测试1: 正转慢速
    Motor_SetDirection(MOTOR_DIR_CW);
    Motor_SetSpeed(300);  // 30%速度   满值1000   
    HAL_Delay(2500);      // 运行2.5秒
    
    // 测试2: 停止
    Motor_Stop();
	
	  float angle1 = Encoder_ReadAngle();
	printf("Current_Angle: %f\r\n",angle1);
	
    // 测试3: 反转慢速
    Motor_SetDirection(MOTOR_DIR_CCW);
    Motor_SetSpeed(300);  // 30%速度
    HAL_Delay(2500);      // 运行2.6秒
    
    // 测试4: 停止
    Motor_Stop();
		
	 float angle2 = Encoder_ReadAngle();
   printf("Current_Angle: %f\r\n",angle2);
}



/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
//主函数
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();   //HAL库初始化

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();   //系统时钟配置

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();       //输入输出引脚初始化
  MX_SPI1_Init();       //SPI初始化
  MX_TIM2_Init();       //定时器2初始化
  MX_USART1_UART_Init();  //串口通信初始化---调试用
  MX_TIM1_Init();     
  MX_ADC1_Init();   
  /* USER CODE BEGIN 2 */
	//编码器读取及电机初始化
  Encoder_Init(&hspi1);     //编码器初始化---配置编码器的spi通信
  Motor_Init(&htim1);       //电机初始化

/////////////////////参数调试处////////主要调节两个环的Kp\Ki\Kd三个值///////////
//位置环PID控制器初始化    Kp    Ki     Kd 
  PID_Init(&position_pid, 8.0f, 0.1f, 0.3f, 100.0f, MAX_MOTOR_SPEED);
//速度环PID控制器初始化  Kp   Ki    Kd 
  PID_Init(&speed_pid, 0.5f, 0.1f, 0.06f, 50.0f, 1000.0f);
////////////////////////////////////////////////////////////////////////////////	

//当前最优参数，作备用
//  PID_Init(&position_pid, 8.0f, 0.1f, 0.3f, 100.0f, MAX_MOTOR_SPEED);
////速度环PID控制器初始化  Kp   Ki    Kd 
//  PID_Init(&speed_pid, 0.5f, 0.1f, 0.06f, 50.0f, 1000.0f);
         
	// 读取上电后的首次角度
	
  float A0 = Encoder_ReadAngle();
	printf("Initial_Angle: %f\r\n",A0);
					
  HAL_Delay(5000);	
	
  //初始角度校准
	for(int i = 0;i < 10; i++){
		Avr[i] = Encoder_ReadAngle();
		sum += Avr[i];
	}
  a = sum/10;
	
	
	printf("%f,%f\r\n", sum,a);
	
	HAL_Delay(100);
	
  //启动定时器
  HAL_TIM_Base_Start_IT(&htim2);  // 控制循环 100Hz
    	
	// 执行简单的电机测试
  // Motor_SimpleTest();
					 
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
		
  }
  /* USER CODE END 3 */
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInit = {0};

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV2;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_ADC;
  PeriphClkInit.AdcClockSelection = RCC_ADCPCLK2_DIV6;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */
//正弦轨迹驱动更新
void UpdateTrajectory(void)
{
    // 更新轨迹时间 (100Hz控制频率，每次增加0.01秒)
    trajectory_t += 0.01f;
    
    // 计算正弦轨迹：-88 * cos(0.1*t) + 90
    // 使用sinf函数计算正弦值
    target_angle = - (TRAJECTORY_AMPLITUDE) * cosf(trajectory_t * 2 * 3.14159f * TRAJECTORY_FREQUENCY) + TRAJECTORY_OFFSET + a;
    
    // 确保角度在0-360度范围内
    if (target_angle >= 360.0f) {
        target_angle -= 360.0f;
    }
    if (target_angle < 0.0f) {
        target_angle += 360.0f;
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
void PositionSpeedDualLoopControl(void)
{
    // 1. 更新速度测量
    Encoder_UpdateVelocity();
    
    // 2. 读取当前位置和当前速度
    float current_angle = Encoder_ReadAngle();
    float current_speed = Encoder_ReadVelocity();
    
    // 3. 位置环：计算位置误差并得到目标速度
    float position_error = target_angle - current_angle;
    if (position_error > 180.0f) position_error -= 360.0f;
    if (position_error < -180.0f) position_error += 360.0f;
    
    // 位置环PID计算，输出为目标速度
    target_speed = PID_Update(&position_pid, 0.0f, position_error);
	  ////////////////////////参数输出添加或删除处/////////////////////////////////////////////////
	  printf("%f,%f,%f,%f,%f\r\n",current_angle,target_angle,current_angle-target_angle,current_speed,target_speed);
    
    // 限制目标速度范围
    if (target_speed > MAX_MOTOR_SPEED) target_speed = MAX_MOTOR_SPEED;
    if (target_speed < -MAX_MOTOR_SPEED) target_speed = -MAX_MOTOR_SPEED;
    
    // 4. 速度环：根据目标速度和实际速度计算PWM
    float speed_error = target_speed - current_speed;
    float pwm_output = PID_Update(&speed_pid, target_speed, current_speed);
    
    // 5. 将PID输出转换为PWM占空比
    uint16_t pwm_duty = (uint16_t)fabsf(pwm_output);
    if (pwm_duty > 1000) pwm_duty = 1000;
    
    // 6. 设置电机方向和速度
    if (target_speed >= 0) {
        Motor_SetDirection(MOTOR_DIR_CW);    //
    } else {
        Motor_SetDirection(MOTOR_DIR_CCW);   //
    }
    Motor_SetSpeed(pwm_duty);
    
}
/////////////////////////////////////////////////////////////////////////////////////////////
// TIM2中断 - 控制循环（100Hz）
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2) {
        control_counter++;  // 统计控制循环次数
        
			  // 更新正弦轨迹目标角度
        UpdateTrajectory();
			
        // 执行位置-速度双环控制
        PositionSpeedDualLoopControl();
        
        // 安全检测
        if (Motor_CheckFault()) {
            // 发生故障，立即停止电机
            Motor_Stop();
        }
    }
}
/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  // 紧急停止电机
  Motor_Stop();
  Motor_Sleep();
	
	__disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
