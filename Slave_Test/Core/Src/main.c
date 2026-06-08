/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
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
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <string.h>

#include "EEPROM.h"
#include "seg7.h"
#include "EEPROM_log.h"
#include "dip.h"
#include "rs485_dummy_slave.h"

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
I2C_HandleTypeDef hi2c1;

UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

//for the interrupt flags
volatile uint8_t inc_flag = 0;
volatile uint8_t rst_flag = 0;
volatile uint8_t clr_flag = 0;

// for the debouncing delay
uint32_t last_inc_time = 0;
uint32_t last_rst_time = 0;
uint32_t last_clr_time = 0;

//counter
uint16_t counter = 0;
//session no.
uint8_t session_no = 0;
//Slave id
uint8_t slave_id;
//i2c status check
HAL_StatusTypeDef i2c_status = HAL_OK;


//for debug
//SessionRecord_t dbg_record = {0};
//uint8_t 		dbg_read_ok = 0;

//SessionRecord_t dbg_old_record = {0};
//uint8_t 		dbg_old_read_ok = 0;

//volatile	uint8_t	dbg_frame_ready = 0;
//uint8_t				dbg_rx_buf[8] = {0};
//uint8_t dbg_slave_rx[8] = {0};
//volatile uint8_t dbg_slave_rx_hit = 0;
//volatile uint16_t dbg_callback_size = 0;
//
//uint8_t dbg_latched = 0;
//uint8_t dbg_latched_rx[8] = {0};
//uint16_t dbg_latched_size = 0;

//debug variables end


/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void MX_I2C1_Init(void);
static void MX_USART3_UART_Init(void);
/* USER CODE BEGIN PFP */

void I2C_BusReset(void);
extern void MX_I2C1_Init(void);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void I2C_BusReset(void){

    // Release I2C peripheral
    HAL_I2C_DeInit(&hi2c1);

    // Configure SCL and SDA as GPIO output temporarily
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin   = GPIO_PIN_6 | GPIO_PIN_7; // PB6=SCL, PB7=SDA
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_OD;      // open drain
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    // Set both high
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6 | GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(5);

    // Clock SCL 9 times to unstick slave device
    for(int i = 0; i < 9; i++){
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_RESET);
        HAL_Delay(1);
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_6, GPIO_PIN_SET);
        HAL_Delay(1);
    }

    // Generate STOP condition
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_RESET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_7, GPIO_PIN_SET);
    HAL_Delay(5);

    // Reinitialize I2C peripheral
    MX_I2C1_Init();
    HAL_Delay(10);
}

/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{

  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_I2C1_Init();
  MX_USART3_UART_Init();
  /* USER CODE BEGIN 2 */

  I2C_BusReset();

  slave_id = DIP_ReadID();

  EEPROM_LOG_Init(&session_no, &counter);

  //RS485_Slave_Init(slave_id);
  //debug
  //HAL_UARTEx_ReceiveToIdle_IT(&huart3, dbg_slave_rx, 8);


  //debug old saved session
  //dbg_old_read_ok = EEPROM_LOG_ReadSession(2, &dbg_old_record);

  SEG7_DisplayNumber(counter);

  //Debug rs485
  RS485_Dummy_Slave_Init(slave_id);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  //debugginh
//	  if(dbg_slave_rx_hit && !dbg_latched){
//
//		  dbg_latched = 1;
//		  dbg_latched_size = dbg_callback_size;
//		  memcpy(dbg_latched_rx, dbg_slave_rx, 8);
//		  //end debug
//	  }

	  if(inc_flag){

		  inc_flag = 0;
		  if(counter < 9999){

			  counter++;
			  EEPROM_LOG_SaveCounter(session_no, counter);

			  //for debugging
			  //dbg_read_ok = EEPROM_LOG_ReadSession(session_no, &dbg_record);

		  }
	  }

	  	  if(rst_flag){

	  	      rst_flag = 0;
	  	      session_no++;
	  	      counter = 0;
	  	      EEPROM_LOG_CommitSession(session_no);

	  	      //for debugging
	  	      //dbg_read_ok = EEPROM_LOG_ReadSession(session_no, &dbg_record);

	  	  }

	  	  // this clr btn is for the end of the day reset.
	  	  if(clr_flag){

	  		  clr_flag = 0;
	  		  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);

	  		  for (int i= 0; i<512; i++){

	  			  EEPROM_PageErase(i);
	  		  }

	  		  HAL_GPIO_TogglePin(LED_GPIO_Port, LED_Pin);
	  		  // Reset state variables
	  		  session_no = 0;
	  		  counter = 0;
	  		  EEPROM_LOG_Init(&session_no, &counter);

	  	  }

	  	  //RS485_Slave_Process(session_no, counter);

	  	  //debug rs485
	  	RS485_Dummy_Slave_Process();

	  	SEG7_DisplayNumber(counter);
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
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
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 400000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief USART3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART3_UART_Init(void)
{

  /* USER CODE BEGIN USART3_Init 0 */

  /* USER CODE END USART3_Init 0 */

  /* USER CODE BEGIN USART3_Init 1 */

  /* USER CODE END USART3_Init 1 */
  huart3.Instance = USART3;
  huart3.Init.BaudRate = 9600;
  huart3.Init.WordLength = UART_WORDLENGTH_8B;
  huart3.Init.StopBits = UART_STOPBITS_1;
  huart3.Init.Parity = UART_PARITY_NONE;
  huart3.Init.Mode = UART_MODE_TX_RX;
  huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart3.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART3_Init 2 */

  /* USER CODE END USART3_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  /* USER CODE BEGIN MX_GPIO_Init_1 */

  /* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LED_GPIO_Port, LED_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, SEG_D2_Pin|SEG_D3_Pin|SEG_D1_Pin|SEG_D4_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : LED_Pin */
  GPIO_InitStruct.Pin = LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(LED_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_A_Pin SEG_B_Pin SEG_C_Pin SEG_D_Pin
                           SEG_E_Pin SEG_F_Pin SEG_G_Pin */
  GPIO_InitStruct.Pin = SEG_A_Pin|SEG_B_Pin|SEG_C_Pin|SEG_D_Pin
                          |SEG_E_Pin|SEG_F_Pin|SEG_G_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : SEG_D2_Pin SEG_D3_Pin SEG_D1_Pin SEG_D4_Pin */
  GPIO_InitStruct.Pin = SEG_D2_Pin|SEG_D3_Pin|SEG_D1_Pin|SEG_D4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : INC_BTN_Pin CLR_BTN_Pin RST_BTN_Pin */
  GPIO_InitStruct.Pin = INC_BTN_Pin|CLR_BTN_Pin|RST_BTN_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_RISING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : DIP0_Pin DIP1_Pin DIP2_Pin DIP3_Pin
                           DIP4_Pin */
  GPIO_InitStruct.Pin = DIP0_Pin|DIP1_Pin|DIP2_Pin|DIP3_Pin
                          |DIP4_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI9_5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI9_5_IRQn);

  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size){

	if(huart->Instance == USART3){

		//
		//debug
		//dbg_slave_rx_hit = 1;
		//dbg_callback_size = size;
		//end debug
		RS485_Dummy_Slave_RxCallback(size);

	}
}


void HAL_GPIO_EXTI_Callback(uint16_t GPIO_PIN){

	uint32_t now = HAL_GetTick();

	if(GPIO_PIN == GPIO_PIN_9){

		if(now - last_rst_time > 500){

			last_rst_time = now;
			rst_flag = 1;
		}
	}
	else if(GPIO_PIN == GPIO_PIN_14){

		if(now - last_clr_time >500){

			last_clr_time = now;
			clr_flag = 1;
		}
	}
	else if(GPIO_PIN == GPIO_PIN_12){

		if(now - last_inc_time > 500){

			last_inc_time = now;
			inc_flag = 1;
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
