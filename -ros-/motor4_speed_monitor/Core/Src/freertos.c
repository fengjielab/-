/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * File Name          : freertos.c
  * Description        : Code for freertos applications
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
#include "FreeRTOS.h"
#include "task.h"
#include "main.h"
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
/* USER CODE BEGIN Includes */

#include "motor.h"
#include "usart.h"
#include <stdio.h>
#include "robot.h"
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
/* USER CODE BEGIN Variables */
static uint8_t uart_rx_byte;

static volatile uint8_t uart_last_cmd = 0;

static volatile uint8_t uart_cmd_ready = 0;


/* ==============================
 * 调试观察变量
 * ============================== */

/* USART实际收到的最后一个字符 */
volatile uint8_t debug_last_rx_cmd = 0;

/* USART接收完成次数 */
volatile uint32_t debug_rx_count = 0;

/* 命令任务处理次数 */
volatile uint32_t debug_cmd_count = 0;

/* UART错误次数 */
volatile uint32_t debug_uart_error_count = 0;

/* USER CODE END Variables */
/* Definitions for defaultTask */
osThreadId_t defaultTaskHandle;
const osThreadAttr_t defaultTask_attributes = {
  .name = "defaultTask",
  .stack_size = 128 * 4,
  .priority = (osPriority_t) osPriorityNormal,
};
/* Definitions for MotorTask */
osThreadId_t MotorTaskHandle;
const osThreadAttr_t MotorTask_attributes = {
  .name = "MotorTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityAboveNormal,
};
/* Definitions for uartTask */
osThreadId_t uartTaskHandle;
const osThreadAttr_t uartTask_attributes = {
  .name = "uartTask",
  .stack_size = 256 * 4,
  .priority = (osPriority_t) osPriorityLow,
};

/* Private function prototypes -----------------------------------------------*/
/* USER CODE BEGIN FunctionPrototypes */

/* USER CODE END FunctionPrototypes */

void StartDefaultTask(void *argument);
void StartMotorTask(void *argument);
void StartUartTask(void *argument);

void MX_FREERTOS_Init(void); /* (MISRA C 2004 rule 8.1) */

/**
  * @brief  FreeRTOS initialization
  * @param  None
  * @retval None
  */
void MX_FREERTOS_Init(void) {
  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* creation of defaultTask */
  defaultTaskHandle = osThreadNew(StartDefaultTask, NULL, &defaultTask_attributes);

  /* creation of MotorTask */
  MotorTaskHandle = osThreadNew(StartMotorTask, NULL, &MotorTask_attributes);

  /* creation of uartTask */
  uartTaskHandle = osThreadNew(StartUartTask, NULL, &uartTask_attributes);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* USER CODE BEGIN RTOS_EVENTS */
  /* add events, ... */
  /* USER CODE END RTOS_EVENTS */

}

/* USER CODE BEGIN Header_StartDefaultTask */
/**
  * @brief  Function implementing the defaultTask thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_StartDefaultTask */
void StartDefaultTask(void *argument)
{
    /* 开始等待USART1接收1个字节 */
    HAL_UART_Receive_IT(
        &huart1,
        &uart_rx_byte,
        1
    );

    for (;;)
    {
        if (uart_cmd_ready)
        {
           debug_cmd_count++;
            uint8_t cmd = uart_last_cmd;

            uart_cmd_ready = 0;

            switch (cmd)
            {
                /* 前进 */
                case 'W':
                case 'w':
                    Robot_Forward(150);
                    break;


                /* 后退 */
                case 'S':
                case 's':
                    Robot_Backward(150);
                    break;


                /* 左移 */
                case 'A':
                case 'a':
                    Robot_Left(150);
                    break;


                /* 右移 */
                case 'D':
                case 'd':
                    Robot_Right(150);
                    break;


                /* 原地左转 */
                case 'Q':
                case 'q':
                    Robot_TurnLeft(150);
                    break;


                /* 原地右转 */
                case 'E':
                case 'e':
                    Robot_TurnRight(150);
                    break;


                /* 停止 */
                case 'X':
                case 'x':
                    Robot_Stop();
                    break;


                default:
                    break;
            }
        }

        osDelay(5);
    }
}

/* USER CODE BEGIN Header_StartMotorTask */
/**
* @brief Function implementing the MotorTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartMotorTask */
void StartMotorTask(void *argument)
{
    TickType_t lastWakeTime;

    lastWakeTime = xTaskGetTickCount();

    for (;;)
    {
        /* 每100ms执行一次完整电机闭环控制 */
        Motor_ControlStep();

        vTaskDelayUntil(
            &lastWakeTime,
            pdMS_TO_TICKS(100)
        );
    }
}

/* USER CODE BEGIN Header_StartUartTask */
/**
* @brief Function implementing the uartTask thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_StartUartTask */
void StartUartTask(void *argument)
{
  /* USER CODE BEGIN StartUartTask */
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END StartUartTask */
}

/* Private application code --------------------------------------------------*/
/* USER CODE BEGIN Application */
void HAL_UART_RxCpltCallback(
    UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 保存刚才收到的字符 */
        uart_last_cmd = uart_rx_byte;

        /* 记录调试信息 */
        debug_last_rx_cmd = uart_rx_byte;
        debug_rx_count++;

        /* 告诉CommandTask有新命令 */
        uart_cmd_ready = 1;


        /* 非常重要：
         * 接完一个字节以后，
         * 马上继续等待下一个字节
         */
        HAL_UART_Receive_IT(
            &huart1,
            &uart_rx_byte,
            1
        );
    }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        HAL_UART_AbortReceive_IT(huart);

        HAL_UART_Receive_IT(
            &huart1,
            &uart_rx_byte,
            1
        );
    }
}
/* USER CODE END Application */

