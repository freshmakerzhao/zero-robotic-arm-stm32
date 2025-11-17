/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    motor_test.h
  * @brief   This file contains all the function prototypes for
  *          the motor_test.c file - Motor and Driver Connection Test Functions
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MOTOR_TEST_H__
#define __MOTOR_TEST_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include <stdint.h>
#include <stdbool.h>

/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* 测试结果类型定义 */
typedef enum {
    MOTOR_TEST_OK       = 0x00U,  /* 测试成功 */
    MOTOR_TEST_ERROR    = 0x01U,  /* 测试失败 */
    MOTOR_TEST_TIMEOUT  = 0x02U,  /* 测试超时 */
    MOTOR_TEST_BUSY     = 0x03U   /* 设备忙 */
} MotorTestStatus_t;

/* 电机测试统计信息 */
typedef struct {
    uint8_t total_count;      /* 总测试数量 */
    uint8_t success_count;    /* 成功数量 */
    uint8_t fail_count;       /* 失败数量 */
    uint8_t timeout_count;    /* 超时数量 */
} MotorTestStats_t;

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* 测试超时时间定义 (ms) */
#define MOTOR_TEST_TIMEOUT_SHORT    1000    /* 短超时 - 1秒 */
#define MOTOR_TEST_TIMEOUT_MEDIUM   3000    /* 中等超时 - 3秒 */
#define MOTOR_TEST_TIMEOUT_LONG     5000    /* 长超时 - 5秒 */

/* 电机地址范围 */
#define MOTOR_ADDR_MIN              1       /* 最小电机地址 */
#define MOTOR_ADDR_MAX              6       /* 最大电机地址（6轴机械臂） */

/* 测试默认参数 */
#define MOTOR_TEST_DEFAULT_SPEED    100     /* 默认测试速度 (RPM) */
#define MOTOR_TEST_DEFAULT_ACC      50      /* 默认测试加速度 */
#define MOTOR_TEST_MOVE_DURATION    500     /* 默认运动持续时间 (ms) */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* 电机地址有效性检查 */
#define IS_MOTOR_ADDR_VALID(addr)   (((addr) >= MOTOR_ADDR_MIN) && ((addr) <= MOTOR_ADDR_MAX))

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/* USER CODE BEGIN EFP */

/**
 * @brief  测试电机连接 - 通过读取固件版本验证连接
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 成功连接, 1: 连接失败
 * 
 * 功能说明：
 * - 向指定地址的电机发送读取固件版本命令
 * - 等待电机响应，验证CAN通信是否正常
 * - 打印接收到的版本信息
 */
int Motor_Test_Connection(uint8_t addr);

/**
 * @brief  测试电机使能功能
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 使能成功, 1: 使能失败
 * 
 * 功能说明：
 * - 发送使能命令到电机驱动器
 * - 检查返回状态码
 * - 验证电机是否正确响应使能命令
 */
int Motor_Test_Enable(uint8_t addr);

/**
 * @brief  测试电机状态读取功能
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 读取成功, 1: 读取失败
 * 
 * 功能说明：
 * - 读取电机的使能/到位/堵转状态标志位
 * - 解析并打印详细的状态信息
 * - 帮助诊断电机当前运行状态
 */
int Motor_Test_ReadStatus(uint8_t addr);

/**
 * @brief  测试电机小幅度运动（速度模式）
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 运动测试成功, 1: 运动测试失败
 * 
 * 功能说明：
 * - 使能电机
 * - 以低速（100 RPM）运行500ms
 * - 停止电机并关闭使能
 * - 用于安全地验证电机是否能正常运动
 * 
 * 注意：
 * - 执行前请确保机械臂处于安全位置
 * - 运动幅度小，适合初次测试
 */
int Motor_Test_SmallMove(uint8_t addr);

/**
 * @brief  批量测试多个电机的连接状态
 * @param  start_addr: 起始电机地址
 * @param  end_addr: 结束电机地址
 * @retval 成功连接的电机数量
 * 
 * 功能说明：
 * - 依次测试指定范围内所有电机的连接
 * - 统计并打印成功连接的电机数量
 * - 适合快速检查所有电机状态
 * 
 * 示例：
 * Motor_Test_All_Connections(1, 6); // 测试地址1到6的所有电机
 */
int Motor_Test_All_Connections(uint8_t start_addr, uint8_t end_addr);

/**
 * @brief  完整测试流程 - 测试单个电机的所有功能
 * @param  addr: 电机地址 (1-6)
 * @retval 失败的测试项数量 (0表示全部通过)
 * 
 * 功能说明：
 * - 依次执行连接、使能、状态读取、运动测试
 * - 每项测试都有详细的进度提示
 * - 统计并报告测试结果
 * - 适合电机和驱动器的全面验证
 * 
 * 测试项目：
 * [1/4] 连接测试
 * [2/4] 使能测试
 * [3/4] 状态读取测试
 * [4/4] 运动测试
 */
int Motor_Test_Complete(uint8_t addr);

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MOTOR_TEST_H__ */