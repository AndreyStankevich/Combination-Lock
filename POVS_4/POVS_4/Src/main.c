#include "main.h"
#include "stdio.h"

#ifdef __GNUC__
    #define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
	#else
	  #define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f) 
#endif	

TIM_HandleTypeDef htim1;
TIM_HandleTypeDef htim2;

UART_HandleTypeDef huart2;

const int PINS[3] = {32, 64, 128}; //column's pins
int counter = 0; //counter for switching logic 1 on keypad columns
int	numberPosition = 0; //numeral index
int number[4]; //array, that contains entered number
int position[4] = {0x100, 0x200, 0x400, 0x800}; //numeral position on controller
int publicPassword[4] = {1, 1, 1, 1};
int privatePassword[4] = {2, 2, 2, 2};
int isChangedPassword = 0; //if == 1, then new password expected

void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_TIM1_Init(void);
static void MX_TIM2_Init(void);
static void MX_USART2_UART_Init(void);
void ResetLCD(void);
void CheckPassword(void);
void ResetLEDS(void);

uint32_t flash_read(uint32_t address) 
{
	return (*(__IO uint32_t*) address);
}
void flash_write(uint32_t address,uint32_t data) 
{
	FLASH->CR |= FLASH_CR_PG;
  *(__IO uint16_t*)address = (uint16_t)data;
	address += 2;
	data >>= 16;
  *(__IO uint16_t*)address = (uint16_t)data;
  FLASH->CR &= ~(FLASH_CR_PG);

}
void flash_unlock(void) 
{
  FLASH->KEYR = (uint32_t)0x45670123;
  FLASH->KEYR = (uint32_t)0xCDEF89AB;
}
void USART2_IRQHandler(void)
{
	int tmp;
  if((USART2->SR & USART_SR_RXNE)!=0)
  {   
		tmp = USART2->DR;
		switch (tmp)
		{
			case '1':
				number[numberPosition++] = 1;
				break;
			case '2':
				number[numberPosition++] = 2;
				break;
			case '3':
				number[numberPosition++] = 3;
				break;
			case '4':
				number[numberPosition++] = 4;
				break;
			case '5':
				number[numberPosition++] = 5;
				break;
			case '6':
				number[numberPosition++] = 6;
				break;
			case '7':
				number[numberPosition++] = 7;
				break;
			case '8':
				number[numberPosition++] = 8;
				break;
			case '9':
				number[numberPosition++] = 9;
				break;
			case '0':
				number[numberPosition++] = 10;
				break;
			case '*':
				ResetLCD();
			  ResetLEDS();
			  isChangedPassword = 0;
				break;
			case '#':
				ResetLCD();
			  ResetLEDS();
			  isChangedPassword = 0;
				break;
		}
		if (numberPosition == 4)
		{
			CheckPassword();
		}
	}
}
void ResetLEDS()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_RESET);
}
void SetLEDS()
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_14, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
}
void WriteZero(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteOne(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteTwo(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteThree(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteFour(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteFive(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteSix(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}

void WriteSeven(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}
void WriteEight(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}
void WriteNine(int numberPosition)
{
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_7, GPIO_PIN_SET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_5, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_6, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, numberPosition, GPIO_PIN_SET);
}
void ResetPins()
{ 
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_8, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, GPIO_PIN_RESET);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_11, GPIO_PIN_RESET);	
}

void CheckPassword()
{
	int isPublicPassword = 1, isPrivatePassword = 1, isCommonPasswords = 1;
	for (int i = 0; i <= 3; i++)
	{
		if (number[i] != publicPassword[i])
		{
			isPublicPassword = 0;
		}
		if (number[i] != privatePassword[i])
		{
			isPrivatePassword = 0;	
		}
	}
	
	for (int i = 0; i <= 3; i++)
	{
		if (number[i] != privatePassword[i])
		{
			isCommonPasswords = 0;
			break;
		}
	}
	
	if (isPublicPassword == 1)
	{
		printf("Correct Password\r\n");
		ResetLEDS();
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, GPIO_PIN_SET);
	}
	else
	{
		if (isPrivatePassword == 0 && isChangedPassword == 0)
		{
			printf("Incorrect Password\r\n");
			ResetLEDS();
		  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_13, GPIO_PIN_SET);
		}
	}
	
	if (isChangedPassword == 1)
	{
		if (isCommonPasswords == 0)
		{
			for (int i = 0; i <= 3; i++)
			{
				publicPassword[i] = number[i];
			}
			printf("Password Update\r\n");
			isChangedPassword = 0;
			ResetLCD();
		}
	}
	
	if (isPrivatePassword == 1)
	{
		printf("Private Password entered\r\n");
		isChangedPassword = 1;
		SetLEDS();
		ResetLCD();
	}
}

void DrawNumbers()
{
	for (int i = 0; i <= 3; i++)
	{
		if (number[i] != NULL)
		{
			switch (number[i])
			{
				case 1:
					WriteOne(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 2:
					WriteTwo(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 3:
					WriteThree(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 4:
					WriteFour(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 5:
					WriteFive(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 6:
					WriteSix(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 7:
					WriteSeven(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 8:
					WriteEight(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 9:
					WriteNine(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
				case 10:
					WriteZero(position[i]);
					HAL_Delay(1);
					HAL_GPIO_WritePin(GPIOA, position[i], GPIO_PIN_RESET);
					break;
			}
		}
	}  
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim->Instance == TIM1) 
  {
    GPIOB->ODR = PINS[counter++];
    if (counter > 2)
    {
			counter = 0;
		}
  }
  if (htim->Instance == TIM2)
	{
		DrawNumbers();
  }	
}

void ClearNumber()
{
	number[0] = NULL;
	number[1] = NULL;
	number[2] = NULL;
	number[3] = NULL;
}

void ResetLCD()
{
	numberPosition = 0;
	ClearNumber();
	ResetPins();
	
	if (isChangedPassword == 0)
	{
		ResetLEDS();
	}
}	

void EXTI0_IRQHandler(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_0) == GPIO_PIN_SET)
	{
		HAL_TIM_Base_Stop(&htim1);
		
		if (numberPosition < 4)
		{
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
			{
				printf("1\r\n");
				number[numberPosition] = 1;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET)
			{
				printf("2\r\n");
				number[numberPosition] = 2;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET)
			{
				printf("3\r\n");
				number[numberPosition] = 3;
				numberPosition++;
			}
		}
		if (numberPosition == 4)
		{
			CheckPassword();
		}
	}
	else
	{
		HAL_TIM_Base_Start(&htim1);
	}
}

void EXTI1_IRQHandler(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_1) == GPIO_PIN_SET)
	{
		HAL_TIM_Base_Stop(&htim1);
		
		if (numberPosition < 4)
		{
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
			{
				printf("4\r\n");
				number[numberPosition] = 4;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET)
			{
				printf("5\r\n");
				number[numberPosition] = 5;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET)
			{
				printf("6\r\n");
				number[numberPosition] = 6;
				numberPosition++;
			}
		}
		if (numberPosition == 4)
		{
			CheckPassword();
		}
	}
	else
	{
		HAL_TIM_Base_Start(&htim1);
	}
}

void EXTI2_IRQHandler(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_2) == GPIO_PIN_SET)
	{
		HAL_TIM_Base_Stop(&htim1);
		
		if (numberPosition < 4)
		{
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
			{
				printf("7\r\n");
				number[numberPosition] = 7;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET)
			{
				printf("8\r\n");
				number[numberPosition] = 8;
				numberPosition++;
			}
			if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET)
			{
				printf("9\r\n");
				number[numberPosition] = 9;
				numberPosition++;
			}
		}
		if (numberPosition == 4)
		{
			CheckPassword();
		}
	}
	else
	{
		HAL_TIM_Base_Start(&htim1);
	}
}

void EXTI3_IRQHandler(void)
{
	if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_3) == GPIO_PIN_SET)
	{
		HAL_TIM_Base_Stop(&htim1);
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_5) == GPIO_PIN_SET)
		{
			printf("*\r\n");
			ResetLCD();
			ResetLEDS();
			isChangedPassword = 0;
		}
		if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7) == GPIO_PIN_SET)
		{
			printf("#\r\n");
			ResetLCD();
			ResetLEDS();
			isChangedPassword = 0;
		}
		if (numberPosition < 4)
		{
		  if (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6) == GPIO_PIN_SET)
		  {
				printf("0\r\n");
			  number[numberPosition] = 10;
			  numberPosition++;
		  }
		}
		if (numberPosition == 4)
		{
			CheckPassword();
		}
	}
	else
	{
		HAL_TIM_Base_Start(&htim1);
	}
}

int main(void)
{
  HAL_Init();
  SystemClock_Config();
  MX_GPIO_Init();
  MX_TIM1_Init();
  MX_TIM2_Init();
  MX_USART2_UART_Init();
	
	HAL_TIM_Base_Start_IT(&htim1);
	HAL_TIM_Base_Start_IT(&htim2);
	
	AFIO->EXTICR[0] |= AFIO_EXTICR1_EXTI0_PB | AFIO_EXTICR1_EXTI1_PB | AFIO_EXTICR1_EXTI2_PB | AFIO_EXTICR1_EXTI3_PB;
	EXTI->RTSR |= EXTI_RTSR_TR0 | EXTI_RTSR_TR1 | EXTI_RTSR_TR2 | EXTI_RTSR_TR3;
	EXTI->FTSR |= EXTI_FTSR_TR0 | EXTI_FTSR_TR1 | EXTI_FTSR_TR2 | EXTI_FTSR_TR3;
	EXTI->PR |= EXTI_PR_PR0 | EXTI_PR_PR1 | EXTI_PR_PR2 | EXTI_PR_PR3;
	EXTI->IMR |= EXTI_IMR_MR0 | EXTI_IMR_MR1 | EXTI_IMR_MR2 | EXTI_IMR_MR3;
  
	NVIC_EnableIRQ(EXTI0_IRQn);
	NVIC_EnableIRQ(EXTI1_IRQn);
	NVIC_EnableIRQ(EXTI2_IRQn);
	NVIC_EnableIRQ(EXTI3_IRQn);
	
	NVIC_EnableIRQ(USART2_IRQn);
	USART2->CR1  |= USART_CR1_TCIE;         
  USART2->CR1  |= USART_CR1_RXNEIE;       
	
	NVIC_SetPriority(EXTI0_IRQn, 1);
	NVIC_SetPriority(EXTI1_IRQn, 1);
	NVIC_SetPriority(EXTI2_IRQn, 1);
	NVIC_SetPriority(EXTI3_IRQn, 1);
	NVIC_SetPriority(TIM2_IRQn, 2);
	NVIC_SetPriority(SysTick_IRQn, 0);
	
	flash_unlock();
	flash_write((uint32_t)0x08007FF0, (uint32_t)1);
	flash_write((uint32_t)0x08007FF1, (uint32_t)2);
	flash_write((uint32_t)0x08007FF2, (uint32_t)3);
	flash_write((uint32_t)0x08007FF3, (uint32_t)4);
}

void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }

  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_0) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM1_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
	
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 7999;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 10;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_TIM2_Init(void)
{
  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 7999;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 2;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_USART2_UART_Init(void)
{
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
}

static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

  /*Configure GPIO pins : PA0 PA1 PA4 PA5 
                           PA6 PA7 PA8 PA9 
                           PA10 PA11 PA12 PA13 
                           PA14 PA15 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_4|GPIO_PIN_5 
                          |GPIO_PIN_6|GPIO_PIN_7|GPIO_PIN_8|GPIO_PIN_9 
                          |GPIO_PIN_10|GPIO_PIN_11|GPIO_PIN_12|GPIO_PIN_13 
                          |GPIO_PIN_14|GPIO_PIN_15;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB5 PB6 PB7 */
  GPIO_InitStruct.Pin = GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

void Error_Handler(void)
{
}

PUTCHAR_PROTOTYPE
{
	HAL_UART_Transmit(&huart2, (uint8_t *)&ch, 1, 0xFFFF);
	return ch;
}
