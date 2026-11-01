#include "encoder.h"
#include <stdio.h>

// 全局变量
Encoder_TypeDef encoder;
extern SPI_HandleTypeDef hspi1;

// 编码器初始化
void Encoder_Init(SPI_HandleTypeDef *hspi)
{
    // 保存SPI句柄
    hspi1 = *hspi;
    
    // 初始化编码器数据结构
    encoder.raw_angle = 0.0f;
    encoder.filtered_angle = 0.0f;
    encoder.velocity = 0.0f;
    encoder.raw_data = 0;
    encoder.last_time = HAL_GetTick();
    encoder.total_ticks = 0;
    encoder.last_angle = 0.0f;
    
    // 拉高片选，初始化SPI通信
    HAL_GPIO_WritePin(ENCODER_CS_PORT, ENCODER_CS_PIN, GPIO_PIN_SET);
    
    // 短暂延时确保编码器稳定
    HAL_Delay(10);
    
}

// 读取原始角度数据
uint16_t Encoder_ReadRawData(void)
{
    uint16_t tx_data = ENCODER_ANGLE_REG;
    uint16_t rx_data = 0;
    
    // 拉低片选开始通信
    HAL_GPIO_WritePin(ENCODER_CS_PORT, ENCODER_CS_PIN, GPIO_PIN_RESET);
    
    // SPI通信 - 读取角度寄存器数据
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx_data, (uint8_t*)&rx_data, 1, HAL_MAX_DELAY);
    
    // 拉高片选结束通信
    HAL_GPIO_WritePin(ENCODER_CS_PORT, ENCODER_CS_PIN, GPIO_PIN_SET);
    
    // 返回原始数据（去掉校验位）
    return rx_data & 0x3FFF;
}

// 读取角度值
float Encoder_ReadAngle(void)
{
    uint16_t raw_data = Encoder_ReadRawData();
    
    // 转换为角度 (0-360度)
    encoder.raw_angle = (raw_data / ENCODER_RESOLUTION) * 360.0f;
    
    // 根据减速比换算为电机输出轴角度
    encoder.filtered_angle = encoder.raw_angle; // / ENCODER_GEAR_RATIO;
    
    // 保存原始数据
    encoder.raw_data = raw_data;
    
    return encoder.filtered_angle;
}

// 更新速度计算
void Encoder_UpdateVelocity(void)
{
    static uint32_t last_update_time = 0;
    uint32_t current_time = HAL_GetTick();
    float time_diff = (current_time - last_update_time) / 1000.0f; // 转换为秒
    
    if (time_diff > 0.000001f) { // 避免除零
        float current_angle = Encoder_ReadAngle();
        float angle_diff = current_angle - encoder.last_angle;
        
        // 处理角度跨越0-360°的情况
        if (angle_diff > 180.0f) angle_diff -= 360.0f;
        if (angle_diff < -180.0f) angle_diff += 360.0f;
        
        // 计算速度 (度/秒)
        encoder.velocity = angle_diff / time_diff;
        
        // 保存当前角度和时间
        encoder.last_angle = current_angle;
        last_update_time = current_time;
    }
}

// 读取速度值
float Encoder_ReadVelocity(void)
{
    return encoder.velocity;
}

// 检查编码器错误
uint8_t Encoder_CheckError(void)
{
    uint16_t tx_data = ENCODER_ERROR_REG;
    uint16_t rx_data = 0;
    
    HAL_GPIO_WritePin(ENCODER_CS_PORT, ENCODER_CS_PIN, GPIO_PIN_RESET);
    HAL_SPI_TransmitReceive(&hspi1, (uint8_t*)&tx_data, (uint8_t*)&rx_data, 1, HAL_MAX_DELAY);
    HAL_GPIO_WritePin(ENCODER_CS_PORT, ENCODER_CS_PIN, GPIO_PIN_SET);
    
    // 检查错误状态位
    return (rx_data & 0x0007) != 0;
}

