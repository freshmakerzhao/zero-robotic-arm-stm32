#include "Emm_V5.h"
#include "can.h"
#include "usart.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

/* 辅助函数：通过UART发送字符串 */
static void UART_Print(const char *str)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)str, strlen(str), HAL_MAX_DELAY);
}

/**
 * @brief  测试电机连接 - 读取驱动器固件版本
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 成功, 1: 失败
 */
int Motor_Test_Connection(uint8_t addr)
{
    char buf[100];
    uint32_t timeout = 1000; // 1秒超时
    uint32_t start_tick = HAL_GetTick();
    
    // 清除接收标志
    can.rxFrameFlag = false;
    
    // 发送读取固件版本命令
    Emm_V5_Read_Sys_Params(addr, S_VER);
    
    // 等待响应
    while(can.rxFrameFlag == false) {
        if ((HAL_GetTick() - start_tick) > timeout) {
            sprintf(buf, "Motor[%d]: Connection FAILED - Timeout\r\n", addr);
            UART_Print(buf);
            return 1;
        }
        HAL_Delay(1);
    }
    
    sprintf(buf, "Motor[%d]: Connection OK!\r\n", addr);
    UART_Print(buf);
    
    sprintf(buf, "Received data: ");
    UART_Print(buf);
    for(int i = 0; i < can.CAN_RxMsg.DLC; i++) {
        sprintf(buf, "0x%02X ", can.rxData[i]);
        UART_Print(buf);
    }
    UART_Print("\r\n");
    
    return 0;
}

/**
 * @brief  测试电机使能功能
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 成功, 1: 失败
 */
int Motor_Test_Enable(uint8_t addr)
{
    char buf[100];
    uint32_t timeout = 1000;
    uint32_t start_tick = HAL_GetTick();
    
    sprintf(buf, "Testing motor[%d] enable...\r\n", addr);
    UART_Print(buf);
    
    // 发送使能命令
    can.rxFrameFlag = false;
    Emm_V5_En_Control(addr, true, false);
    
    // 等待响应
    while(can.rxFrameFlag == false) {
        if ((HAL_GetTick() - start_tick) > timeout) {
            sprintf(buf, "Motor[%d]: Enable FAILED - Timeout\r\n", addr);
            UART_Print(buf);
            return 1;
        }
        HAL_Delay(1);
    }
    
    // 检查返回状态
    if (can.rxData[2] == 0x00) {
        sprintf(buf, "Motor[%d]: Enable OK\r\n", addr);
        UART_Print(buf);
        return 0;
    } else {
        sprintf(buf, "Motor[%d]: Enable FAILED, Status: 0x%02X\r\n", addr, can.rxData[2]);
        UART_Print(buf);
        return 1;
    }
}

/**
 * @brief  测试电机状态读取
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 成功, 1: 失败
 */
int Motor_Test_ReadStatus(uint8_t addr)
{
    char buf[100];
    uint32_t timeout = 1000;
    uint32_t start_tick = HAL_GetTick();
    
    sprintf(buf, "Reading motor[%d] status...\r\n", addr);
    UART_Print(buf);
    
    // 读取使能/到位/堵转状态标志位
    can.rxFrameFlag = false;
    Emm_V5_Read_Sys_Params(addr, S_FLAG);
    
    // 等待响应
    while(can.rxFrameFlag == false) {
        if ((HAL_GetTick() - start_tick) > timeout) {
            sprintf(buf, "Motor[%d]: Read status FAILED - Timeout\r\n", addr);
            UART_Print(buf);
            return 1;
        }
        HAL_Delay(1);
    }
    
    if (can.CAN_RxMsg.DLC >= 3) {
        uint8_t status = can.rxData[2];
        sprintf(buf, "Motor[%d] Status:\r\n", addr);
        UART_Print(buf);
        
        sprintf(buf, "  - Enable: %s\r\n", (status & 0x01) ? "Enabled" : "Disabled");
        UART_Print(buf);
        
        sprintf(buf, "  - Position: %s\r\n", (status & 0x02) ? "In Position" : "Moving");
        UART_Print(buf);
        
        sprintf(buf, "  - Stall: %s\r\n", (status & 0x04) ? "Stalled" : "Normal");
        UART_Print(buf);
        
        return 0;
    }
    
    sprintf(buf, "Motor[%d]: Read status FAILED\r\n", addr);
    UART_Print(buf);
    return 1;
}

/**
 * @brief  测试电机小幅度运动
 * @param  addr: 电机地址 (1-6)
 * @retval 0: 成功, 1: 失败
 */
int Motor_Test_SmallMove(uint8_t addr)
{
    char buf[100];
    uint32_t timeout = 5000; // 5秒超时
    uint32_t start_tick = HAL_GetTick();
    
    sprintf(buf, "Testing motor[%d] movement (velocity mode)...\r\n", addr);
    UART_Print(buf);
    
    // 先使能电机
    if (Motor_Test_Enable(addr) != 0) {
        return 1;
    }
    
    HAL_Delay(100);
    
    // 发送速度命令 - 正向，100RPM，加速度50，运行500ms
    can.rxFrameFlag = false;
    Emm_V5_Vel_Control(addr, 0, 100, 50, false);
    
    // 等待响应
    while(can.rxFrameFlag == false) {
        if ((HAL_GetTick() - start_tick) > timeout) {
            sprintf(buf, "Motor[%d]: Move command FAILED\r\n", addr);
            UART_Print(buf);
            return 1;
        }
        HAL_Delay(1);
    }
    
    sprintf(buf, "Motor[%d]: Moving...\r\n", addr);
    UART_Print(buf);
    HAL_Delay(500); // 运行500ms
    
    // 停止电机
    can.rxFrameFlag = false;
    Emm_V5_Stop_Now(addr, false);
    
    start_tick = HAL_GetTick();
    while(can.rxFrameFlag == false) {
        if ((HAL_GetTick() - start_tick) > timeout) {
            sprintf(buf, "Motor[%d]: Stop command FAILED\r\n", addr);
            UART_Print(buf);
            return 1;
        }
        HAL_Delay(1);
    }
    
    sprintf(buf, "Motor[%d]: Movement test completed\r\n", addr);
    UART_Print(buf);
    
    // 关闭使能
    Emm_V5_En_Control(addr, false, false);
    HAL_Delay(100);
    
    return 0;
}

/**
 * @brief  批量测试所有电机连接
 * @param  start_addr: 起始地址
 * @param  end_addr: 结束地址
 * @retval 成功连接的电机数量
 */
int Motor_Test_All_Connections(uint8_t start_addr, uint8_t end_addr)
{
    char buf[100];
    int success_count = 0;
    
    UART_Print("\r\n========== Batch Connection Test Start ==========\r\n");
    
    for (uint8_t addr = start_addr; addr <= end_addr; addr++) {
        if (Motor_Test_Connection(addr) == 0) {
            success_count++;
        }
        HAL_Delay(100); // 每次测试间隔
    }
    
    UART_Print("\r\n========== Test Completed ==========\r\n");
    sprintf(buf, "Successfully connected: %d/%d\r\n", success_count, (end_addr - start_addr + 1));
    UART_Print(buf);
    
    return success_count;
}

/**
 * @brief  完整测试流程 - 测试单个电机的所有功能
 * @param  addr: 电机地址
 * @retval 0: 全部成功, 其他: 失败数量
 */
int Motor_Test_Complete(uint8_t addr)
{
    char buf[100];
    int fail_count = 0;
    
    sprintf(buf, "\r\n========== Complete Test for Motor[%d] Start ==========\r\n", addr);
    UART_Print(buf);
    
    // 1. 连接测试
    UART_Print("\r\n[1/4] Connection Test...\r\n");
    if (Motor_Test_Connection(addr) != 0) {
        fail_count++;
        UART_Print("Connection test FAILED, skip remaining tests\r\n");
        return fail_count;
    }
    HAL_Delay(200);
    
    // 2. 使能测试
    UART_Print("\r\n[2/4] Enable Test...\r\n");
    if (Motor_Test_Enable(addr) != 0) {
        fail_count++;
    }
    HAL_Delay(200);
    
    // 3. 状态读取测试
    UART_Print("\r\n[3/4] Status Reading Test...\r\n");
    if (Motor_Test_ReadStatus(addr) != 0) {
        fail_count++;
    }
    HAL_Delay(200);
    
    // 4. 运动测试
    UART_Print("\r\n[4/4] Movement Test...\r\n");
    if (Motor_Test_SmallMove(addr) != 0) {
        fail_count++;
    }
    
    UART_Print("\r\n========== Test Completed ==========\r\n");
    if (fail_count == 0) {
        sprintf(buf, "Motor[%d]: All tests PASSED!\r\n", addr);
        UART_Print(buf);
    } else {
        sprintf(buf, "Motor[%d]: %d test(s) FAILED\r\n", addr, fail_count);
        UART_Print(buf);
    }
    
    return fail_count;
}