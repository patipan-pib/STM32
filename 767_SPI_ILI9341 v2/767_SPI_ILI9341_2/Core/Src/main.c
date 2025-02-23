/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "adc.h"
#include "i2c.h"
#include "rng.h"
#include "spi.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "stdio.h"
#include "ILI9341_Touchscreen.h"
#include "ILI9341_STM32_Driver.h"
#include "ILI9341_GFX.h"
#include "snow_tiger.h"
#include "my_image.h"

#include "adc.h"
#include "tim.h"

extern I2C_HandleTypeDef hi2c1;  // ใช้ I2C1
uint8_t data_buffer[8];
#define AM2320_ADDR 0xB8  // I2C Address ของ AM2320 (Shifted 8-bit)
uint16_t temp_raw;
#define MAX_PERCENT 100
#define STEP_PERCENT 10
float temperature = 0.0;
float humidity = 0.0;
uint16_t ILI9341_Color565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}


uint8_t red_percent = 0;
uint8_t green_percent = 0;
uint8_t blue_percent = 0;

uint8_t current_screen = 1; // 1 = หน้าหลัก, 2 = หน้าจอที่สอง
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
/* Private variables ---------------------------------------------------------*/

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */
/* Private function prototypes -----------------------------------------------*/
void draw_color_buttons();
void update_percentage_bar(uint16_t button_id);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

  char str[50];
  uint8_t cmdBuffer[3] ={0x03,0x00,0x04};
  uint8_t dataBuffer[8];


void read_AM2320(float *temperature, float *humidity) {
			HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmdBuffer, 3, 100);

		  HAL_I2C_Master_Transmit(&hi2c1, 0x5c << 1, cmdBuffer, 3, 100);
		  HAL_Delay(1);

		  HAL_I2C_Master_Receive(&hi2c1, 0x5c << 1, dataBuffer, 8, 100);

		  uint16_t t = ((dataBuffer[4] & 0x7F) << 8)+ dataBuffer[5];
		  *temperature = t/10.0;
//		  t = (((dataBuffer[4] & 0x80 )>> 7) == 1) ? (t*(-1)) :t ;

		  uint16_t h = (dataBuffer[2] << 8) + dataBuffer[3];
		  *humidity = h/10.0;
}

void display_sensor_data();  // ประกาศฟังก์ชันก่อนใช้งาน

// ฟังก์ชัน printf() ใช้ UART3 แทน UART1
int _write(int file, char *ptr, int len) {
    HAL_UART_Transmit(&huart3, (uint8_t*) ptr, len, 100);
    return len;
}


void adjust_backlight(uint16_t adc_value) {
    // จำกัดค่า ADC ให้อยู่ในช่วงที่ถูกต้อง
    if (adc_value > 4095) adc_value = 4095;

    // แปลงค่า ADC (0 - 4095) ให้เป็นช่วง 20% - 100%
    uint8_t brightness = (adc_value * 80 / 4095) + 20;

    // คำนวณ Duty Cycle ตามค่า brightness
    uint32_t duty_cycle = (htim1.Init.Period * brightness) / 100;

    // ตั้งค่า PWM Output
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_1, duty_cycle);
}



/* ฟังก์ชันที่ใช้เพื่ออัปเดตเปอร์เซ็นต์ของแต่ละสี */
//void ILI9341_Draw_Scaled_Image(uint16_t x, uint16_t y, uint16_t new_width, uint16_t new_height, uint16_t orig_width, uint16_t orig_height, const uint16_t* data) {
//    uint16_t scale_x = orig_width / new_width;
//    uint16_t scale_y = orig_height / new_height;
//
//    for (uint16_t j = 0; j < new_height; j++) {
//        for (uint16_t i = 0; i < new_width; i++) {
//            uint16_t pixel = data[(j * scale_y * orig_width) + (i * scale_x)];
//            ILI9341_Draw_Pixel(x + i, y + j, pixel);
//        }
//    }
//}

void ILI9341_Draw_Image_Scaled(uint16_t x, uint16_t y,
                               uint16_t new_width, uint16_t new_height,
                               uint16_t orig_width, uint16_t orig_height,
                               const uint16_t* image)
{
    // 1. Allocate a temporary buffer for the scaled image.
    //    (This buffer is sized to hold new_width * new_height pixels.)
    //    Note: If new_width*new_height is too big for the stack,
    //    consider using dynamic memory (malloc) or a statically allocated array.
    uint16_t scaled_image[new_width * new_height];

    // 2. Calculate scaling factors (using floating point for better accuracy)
    float scale_x = (float)orig_width / new_width;
    float scale_y = (float)orig_height / new_height;

    // 3. Fill the scaled_image buffer using nearest-neighbor scaling.
    for (uint16_t j = 0; j < new_height; j++) {
        for (uint16_t i = 0; i < new_width; i++) {
            // Calculate the corresponding original pixel coordinates.
            uint16_t orig_x = (uint16_t)(i * scale_x);
            uint16_t orig_y = (uint16_t)(j * scale_y);
            scaled_image[j * new_width + i] = image[orig_y * orig_width + orig_x];
        }
    }

    // 4. Set the display area where the scaled image will be drawn.
    //    Note: Adjust ILI9341_Set_Address parameters if your function expects (x0,y0,x1,y1).
    ILI9341_Set_Address(x, y, x + new_width - 1, y + new_height - 1);

    // 5. Prepare to send pixel data via SPI in bursts.
    HAL_GPIO_WritePin(GPIOC, DC_Pin, GPIO_PIN_SET);
    HAL_GPIO_WritePin(GPIOC, CS_Pin, GPIO_PIN_RESET);

    // 6. Calculate total number of bytes to send (2 bytes per pixel).
    uint32_t total_bytes = new_width * new_height * 2;
    uint32_t counter = 0;
    unsigned char temp_buffer[BURST_MAX_SIZE];

    // 7. Send the scaled image data in bursts.
    while (counter < total_bytes) {
        // Determine how many bytes to send in this burst.
        uint32_t chunk_size = ((total_bytes - counter) < BURST_MAX_SIZE) ? (total_bytes - counter) : BURST_MAX_SIZE;
        // Copy the data from the scaled image into the temporary buffer.
        // (We cast scaled_image to a pointer to bytes.)
        memcpy(temp_buffer, ((uint8_t*)scaled_image) + counter, chunk_size);
        // Transmit the chunk over SPI.
        HAL_SPI_Transmit(&hspi5, temp_buffer, chunk_size, 10);
        counter += chunk_size;
    }

    HAL_GPIO_WritePin(GPIOC, CS_Pin, GPIO_PIN_SET);
}


void update_percentage_bar(uint16_t button_id) {
	char buffer[20];
	// แสดงค่าอุณหภูมิ
	    sprintf(buffer, "%.1fC", temperature);
	    ILI9341_Draw_Text(buffer, 10, 10, BLACK, 2, WHITE);

	    // แสดงค่าความชื้นสัมพัทธ์
	    sprintf(buffer, "%.1f%%RH", humidity);
	    ILI9341_Draw_Text(buffer, 160, 10, BLACK, 2, WHITE);

    // ผสมสี
	    uint16_t mixed_color = ILI9341_Color565(red_percent * 2.55, green_percent * 2.55, blue_percent * 2.55);
	    ILI9341_Draw_Filled_Circle(120, 20, 15, mixed_color);

    // เพิ่มเปอร์เซ็นต์ตามปุ่มที่กด
    if (button_id == 1 && red_percent < MAX_PERCENT) {
        red_percent += STEP_PERCENT;
    } else if (button_id == 2 && green_percent < MAX_PERCENT) {
        green_percent += STEP_PERCENT;
    } else if (button_id == 3 && blue_percent < MAX_PERCENT) {
        blue_percent += STEP_PERCENT;
    }

    // อัปเดตแถบสี
        char text_buffer[30];

        // แถบสีแดง
        uint16_t red_shade = ILI9341_Color565(red_percent * 2.55, 0, 0);
        sprintf(text_buffer, "%d%%", red_percent);
        ILI9341_Draw_Text(text_buffer, 210, 40, BLACK, 2, WHITE);
        ILI9341_Draw_Filled_Circle(20, 50, 10, RED);
        ILI9341_Draw_Rectangle(40, 40, 160, 20, BLACK);
        ILI9341_Draw_Filled_Rectangle_Coord(40, 40, 40 + (red_percent * 1.6), 60, red_shade);

        // แถบสีเขียว
        uint16_t green_shade = ILI9341_Color565(0, green_percent * 2.55, 0);
        sprintf(text_buffer, "%d%%", green_percent);
        ILI9341_Draw_Text(text_buffer, 210, 70, BLACK, 2, WHITE);
        ILI9341_Draw_Filled_Circle(20, 80, 10, GREEN);
        ILI9341_Draw_Rectangle(40, 70, 160, 20, BLACK);
        ILI9341_Draw_Filled_Rectangle_Coord(40, 70, 40 + (green_percent * 1.6), 90, green_shade);

        // แถบสีน้ำเงิน
        uint16_t blue_shade = ILI9341_Color565(0, 0, blue_percent * 2.55);
        sprintf(text_buffer, "%d%%", blue_percent);
        ILI9341_Draw_Text(text_buffer, 210, 100, BLACK, 2, WHITE);
        ILI9341_Draw_Filled_Circle(20, 110, 10, BLUE);
        ILI9341_Draw_Rectangle(40, 100, 160, 20, BLACK);
        ILI9341_Draw_Filled_Rectangle_Coord(40, 100, 40 + (blue_percent * 1.6), 120, blue_shade);

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

  /* Enable the CPU Cache */

  /* Enable I-Cache---------------------------------------------------------*/
  SCB_EnableICache();

  /* Enable D-Cache---------------------------------------------------------*/
  SCB_EnableDCache();

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
  MX_USART3_UART_Init();
  MX_SPI5_Init();
  MX_TIM1_Init();
  MX_RNG_Init();
  MX_I2C1_Init();
  MX_ADC1_Init();
  /* USER CODE BEGIN 2 */
  ILI9341_Init();
  ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);  // หมุนหน้าจอเป็นแนวนอน
  ILI9341_Fill_Screen(WHITE); // เติมหน้าจอด้วยสีขาว
  update_percentage_bar(0);

  adjust_backlight(50);  // ปรับเป็น 50%
  // กำหนดพิกัดของปุ่ม
  uint16_t button_radius = 15;
  uint16_t button_y = 30;

  uint16_t button1_x = 190;
  uint16_t button1_y = button_y;

  uint16_t button2_x = 170;
  uint16_t button2_y = button_y;

  uint16_t button3_x = 120;
  uint16_t button3_y = button_y;

  uint32_t last_touch_time = 0;  // เก็บเวลาของการสัมผัสครั้งล่าสุด
  uint32_t timeout_duration = 5000;  // เวลาที่จะให้หน้าจอกลับ (5 วินาที)


  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {

    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */

	  char buffer[20];
	  read_AM2320(&temperature, &humidity);
	  sprintf(buffer, "%.1fC", temperature);
	  ILI9341_Draw_Text(buffer, 10, 10, BLACK, 2, WHITE);
	  sprintf(buffer, "%.1f%%RH", humidity);
	  ILI9341_Draw_Text(buffer, 160, 10, BLACK, 2, WHITE);
	  HAL_Delay(20);  // รอเวลาสั้นๆ เพื่อไม่ให้โค้ดทำงานหนักเกินไป



	     if (TP_Touchpad_Pressed()) {
	         uint16_t x_pos = 0;
	         uint16_t y_pos = 0;
	         uint16_t position_array[2];

	         if (TP_Read_Coordinates(position_array) == TOUCHPAD_DATA_OK) {
	             x_pos = position_array[0];
	             y_pos = position_array[1];

	             // อัปเดตเวลาของการสัมผัสครั้งล่าสุด
	             last_touch_time = HAL_GetTick();

	             // ตรวจสอบว่ากดปุ่มที่ตำแหน่งใด
	             int distance_x, distance_y;

	             // ปุ่มที่ 1 (สีแดง)
	             distance_x = abs(x_pos - button1_x);
	             distance_y = abs(y_pos - button1_y);
	             if (distance_x <= button_radius && distance_y <= button_radius) {
	                 update_percentage_bar(1);
	                 HAL_Delay(200);  // ลดการตอบสนองหลายครั้ง
	             }

	             // ปุ่มที่ 2 (สีเขียว)
	             distance_x = abs(x_pos - button2_x);
	             distance_y = abs(y_pos - button2_y);
	             if (distance_x <= button_radius && distance_y <= button_radius) {
	                 update_percentage_bar(2);
	                 HAL_Delay(200);
	             }

	             // ปุ่มที่ 3 (สีน้ำเงิน)
	             distance_x = abs(x_pos - button3_x);
	             distance_y = abs(y_pos - button3_y);
	             if (distance_x <= button_radius && distance_y <= button_radius) {
	                 update_percentage_bar(3);
	                 HAL_Delay(200);
	             }



	             // จัดการการเปลี่ยนหน้าจอ
	             if (current_screen == 1) {
	                 if (abs(x_pos - 220) <= 15 && abs(y_pos - 120) <= 15) {
	                     current_screen = 2;
	                     ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);  // หมุนหน้าจอเป็นแนวนอน
	                     ILI9341_Fill_Screen(WHITE);  // ล้างหน้าจอ

	                     // วาดรูปภาพหรือเนื้อหาของหน้าจอที่สอง
	                     ILI9341_Draw_Image_Scaled(10, 40,90,128, 320, 240, snow_tiger);
//	                     ILI9341_Draw_Image((const char*)snow_tiger, SCREEN_HORIZONTAL_1);

	                     // แสดงข้อมูล
	                     ILI9341_Draw_Text("Group No.1", 120, 40, MAGENTA, 2, WHITE);
	                     ILI9341_Draw_Text("Jirawat", 120, 80, MAGENTA, 2, WHITE);
	                     ILI9341_Draw_Text("Patipan", 120, 110, MAGENTA, 2, WHITE);
	                     ILI9341_Draw_Text("66015116", 120, 140, MAGENTA, 2, WHITE);

	                     HAL_Delay(200);
	                 }
	                 else if (abs(x_pos - 190) <= 15 && abs(y_pos - 30) <= 15)
	                     update_percentage_bar(1);
	                 else if (abs(x_pos - 170) <= 15 && abs(y_pos - 30) <= 15)
	                     update_percentage_bar(2);
	                 else if (abs(x_pos - 120) <= 15 && abs(y_pos - 30) <= 15)
	                     update_percentage_bar(3);
	             }
	             else if (current_screen == 2) {
	                 if (abs(x_pos - 160) <= 60 && abs(y_pos - 60) <= 35) {
	                     current_screen = 1;
	                     ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);  // หมุนหน้าจอเป็นแนวนอน
	                     ILI9341_Fill_Screen(WHITE);  // ล้างหน้าจอ

	                     update_percentage_bar(0);
	                     HAL_Delay(200);
	                 }
	             }
	         }
	     }

	     // เช็คว่าเวลาผ่านไป 5 วินาทีหรือไม่ในหน้าจอที่สอง
	     if (current_screen == 2 && (HAL_GetTick() - last_touch_time) >= timeout_duration) {
	         current_screen = 1;
	         ILI9341_Set_Rotation(SCREEN_HORIZONTAL_1);  // หมุนหน้าจอเป็นแนวนอน
	         ILI9341_Fill_Screen(WHITE);  // ล้างหน้าจอ
	         update_percentage_bar(0);
	         HAL_Delay(200);
	     }



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

  /** Configure LSE Drive Capability
  */
  HAL_PWR_EnableBkUpAccess();

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_BYPASS;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 200;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  /** Activate the Over-Drive mode
  */
  if (HAL_PWREx_EnableOverDrive() != HAL_OK)
  {
    Error_Handler();
  }

  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_6) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */
  while(1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
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
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */
