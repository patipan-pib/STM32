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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <string.h>
#include <stdio.h>
char rx_buffer;
char rx_message[100];
char rx_msg_external[100];
char rx_msg_ex_coppy[100];
char tx_message[100];
char name_1[50];
char name_2[50];
int len_rx;
int len;
int status;
uint8_t newLine[] = "\r\n";
UART_HandleTypeDef *selected_uart;

// set UART number Start UART1 or URAT2
int uart_no = 1;
// set UART number End

// .
// .
// .

/* Private variables ---------------------------------------------------------*/

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart2;
UART_HandleTypeDef huart3;

/* USER CODE BEGIN PV */

// .
// .
// .

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (status != -1) {
//    	memset(rx_msg_external, 0, sizeof(rx_msg_external));
        HAL_UART_Receive_IT(selected_uart, (uint8_t *)rx_msg_external, sizeof(rx_msg_external));
        status = 1;
    }
}

void formatStr()
{
	int start_index = 0;

	while (rx_msg_external[start_index] == '\0' && start_index < sizeof(rx_msg_external))
	{
		start_index++;
	}

	if (start_index < sizeof(rx_msg_external))
	{
		strcpy(rx_msg_ex_coppy, &rx_msg_external[start_index]);

		if (strlen(name_2) == 0)
		{
			strcpy(name_2, rx_msg_ex_coppy);
			memset(tx_message, 0, sizeof(tx_message));
			snprintf(tx_message, sizeof(tx_message), "%s is ready\r\n", name_2);
			HAL_UART_Transmit(&huart3, (uint8_t *)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
			status = 2;
		}
		else if (strcmp(rx_msg_ex_coppy, "RXhpdCBQcm9ncmFtICEh") == 0)
		{
			memset(tx_message, 0, sizeof(tx_message));
			snprintf(tx_message, sizeof(tx_message), "Exit Program !!");
			HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
			status = -1;
		}
		else
		{
			HAL_UART_Transmit(&huart3, (uint8_t *)rx_msg_ex_coppy, sizeof(rx_msg_ex_coppy), HAL_MAX_DELAY);
			HAL_UART_Transmit(&huart3, newLine, sizeof(newLine), HAL_MAX_DELAY);
			status = 2;
		}
	}
}

void title_line()
{
	memset(tx_message, 0, sizeof(tx_message));
	if (strlen(name_1) == 0)
	{
		HAL_UART_Transmit(&huart3, (uint8_t*)"NAME : ", 7, HAL_MAX_DELAY);
	}
	else
	{
		snprintf(tx_message, sizeof(tx_message), "%s => ", name_1);
	    HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
	}

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

  /* MPU Configuration--------------------------------------------------------*/
   MPU_Config();

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
  MX_USART2_UART_Init();
  MX_USART3_UART_Init();
  MX_USART1_UART_Init();
  /* USER CODE BEGIN 2 */


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */


	HAL_UART_Transmit(&huart3, newLine, sizeof(newLine), HAL_MAX_DELAY);

	snprintf(tx_message, sizeof(tx_message),
		  "Man from U.A.R.T.%lu !\r\n"
		  "Quit PRESS q\r\n", uart_no);
	HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);

	memset(tx_message, 0, sizeof(tx_message));
	memset(rx_msg_external, 0, sizeof(rx_msg_external));

	if (uart_no == 1)
	{
		selected_uart = &huart1;
		title_line();
		status = 2;
	}
	else if (uart_no == 2)
	{
		selected_uart = &huart2;
		status = 0;
	}
	else
	{
		return 0;
	}

	HAL_UART_Receive_IT(selected_uart, rx_msg_external, sizeof(rx_msg_external));



  while (status != -1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  if (status == 1) {
		  formatStr();
		  if (status != -1)
		  {

			  title_line();
		  }
	  }
	  else if (status == 2) {

		  // input char
		  if (HAL_UART_Receive(&huart3, (uint8_t*)&rx_buffer, sizeof(rx_buffer), HAL_MAX_DELAY) == HAL_OK)
		  {
			  HAL_UART_Transmit(&huart3, (uint8_t*)&rx_buffer, sizeof(rx_buffer), HAL_MAX_DELAY);

			  // add char to string
			  len = strlen(rx_message);
			  if (len < sizeof(rx_message) - 1 && rx_buffer != '\r')
			  {
				  rx_message[len] = rx_buffer;
				  rx_message[len + 1] = '\0';

			  }
			  len_rx = strlen(rx_message);


			  // action ENTER KEY
			  if (rx_buffer == '\r')
			  {
				  if (strlen(rx_message) == 1 && (strcmp(rx_message, "q") == 0 || strcmp(rx_message, "Q") == 0))
				  {
					  // Exit Program !! => encode base64 UFT8 RXhpdCBQcm9ncmFtICEh
					  memset(tx_message, 0, sizeof(tx_message));
					  snprintf(tx_message, sizeof(tx_message), "RXhpdCBQcm9ncmFtICEh");
					  HAL_UART_Transmit(selected_uart, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
					  HAL_UART_Transmit(&huart3, newLine, sizeof(newLine), HAL_MAX_DELAY);

					  memset(tx_message, 0, sizeof(tx_message));
					  snprintf(tx_message, sizeof(tx_message), "Exit Program !!");
					  HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
					  status = -1;
				  }
				  else if (strlen(name_1) == 0 && len_rx > 0 )
				  {
					  strncpy(name_1, rx_message, sizeof(name_1)-1);
					  name_1[sizeof(name_1) - 1] = '\0';

					  // EXTERNAL BOARD START

					  memset(tx_message, 0, sizeof(tx_message));
					  snprintf(tx_message, sizeof(tx_message), "%s", name_1);
					  HAL_UART_Transmit(selected_uart, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
					  HAL_UART_Transmit(&huart3, newLine, sizeof(newLine), HAL_MAX_DELAY);

					  if (uart_no == 2)
					  {
						  memset(tx_message, 0, sizeof(tx_message));
						  snprintf(tx_message, sizeof(tx_message), "%s : ", name_2);
						  HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
					  }

					  status = 0;

					  // EXTERNAL BOARD END

				  }
				  else if (strlen(name_1) == 0 && len_rx == 0 )
				  {
					  title_line();
					  status = 2;
				  }
				  else if (strlen(name_1) != 0)
				  {
					  // EXTERNAL BOARD START
					  memset(tx_message, 0, sizeof(tx_message));

					  snprintf(tx_message, sizeof(tx_message), "%s", rx_message);
					  HAL_UART_Transmit(selected_uart, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);
					  HAL_UART_Transmit(&huart3, newLine, sizeof(newLine), HAL_MAX_DELAY);

					  memset(tx_message, 0, sizeof(tx_message));

					  snprintf(tx_message, sizeof(tx_message), "%s : ", name_2);
					  HAL_UART_Transmit(&huart3, (uint8_t*)tx_message, sizeof(tx_message), HAL_MAX_DELAY);

					  status = 0;

					  // EXTERNAL BOARD END
				  }

				  len_rx = 0;
				  memset(rx_message, 0, sizeof(rx_message));

			  }
		  }
	  }
  }
  /* USER CODE END 3 */
}
// .
// .
// .