/* USER CODE BEGIN Header */

//używana płytka STM32: NUCLEO-F072RB
//TODO: Regulator PID
//TODO: Dodać multiplier

/* USER CODE END Header */
/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

//część napisana przez Tomka Andrzejewskiego

uint8_t enginesFrame[5];				// Ramka, do ktorej UART wpisuje odbierane dane dot. silnikow

struct EnginesData{						// Struktura przechowujaca skonwertowane dane dla silnikow
	int multiplier;
	int leftPower, rightPower;			//wartość 0-9, zmieniana w dalszej części kodu na wartość PWM
	int leftDirection, rightDirection;	//wartośc 0-1
	}enginesData;

	int rightPower;						//przypisywanie wartości PWM
	int leftPower;						//Przypisywanie wartości PWM



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

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_TIM3_Init();
  /* USER CODE BEGIN 2 */
  HAL_UART_Receive_IT(&huart2, enginesFrame, 5);	// Inicjalizacja odbierania danych przez UART w trybie przerwaniowym

  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_2);			//Inicjalizacja timerów do PWM
  HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)															 //M1 - lewy silnik; M2 - prawy silnik
  {

	 rightPower=enginesData.rightPower*25;							//zmiana wartości 0-9 wziętej ze struktury na sygnał PWM 0-255 [leftPower, rightPower]
	 leftPower=enginesData.leftPower*25;

  if(enginesData.leftDirection==1 && enginesData.rightDirection==0)		//skręt w prawo
	{
	 	HAL_GPIO_WritePin(M1_GPIO_Port,M1_Pin,GPIO_PIN_SET);
	 	HAL_GPIO_WritePin(M2_GPIO_Port,M2_Pin,GPIO_PIN_RESET);			//analogicznie niżej
	}
  else if(enginesData.leftDirection==0 && enginesData.rightDirection==1)	//skręt w lewo
  	{
	  	HAL_GPIO_WritePin(M2_GPIO_Port,M2_Pin,GPIO_PIN_SET);
	 	HAL_GPIO_WritePin(M1_GPIO_Port,M1_Pin,GPIO_PIN_RESET);
  	}
  else if(enginesData.leftDirection==1 && enginesData.rightDirection==1) //do przodu
  	{
	 	HAL_GPIO_WritePin(M1_GPIO_Port,M1_Pin,GPIO_PIN_SET);
	 	HAL_GPIO_WritePin(M2_GPIO_Port,M2_Pin,GPIO_PIN_SET);
	}
  else if(enginesData.leftDirection==0 && enginesData.rightDirection==0) //do tyłu
	{
	 	HAL_GPIO_WritePin(M1_GPIO_Port,M1_Pin,GPIO_PIN_RESET);
	 	HAL_GPIO_WritePin(M2_GPIO_Port,M2_Pin,GPIO_PIN_RESET);
	}
  TIM3->CCR1=rightPower;			//mialo byc bez mnoznikow wiec jest sama wartosc rightPower odebrana z uart
  TIM3->CCR2=leftPower;

//TODO: Ewentualna optymalizacja

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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI48;
  RCC_OscInitStruct.HSI48State = RCC_HSI48_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_HSI48;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_1) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_USART2;
  PeriphClkInit.Usart2ClockSelection = RCC_USART2CLKSOURCE_PCLK1;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit) != HAL_OK)
  {
    Error_Handler();
  }
}

/* USER CODE BEGIN 4 */

void convertEnginesFrame(){		// Konwertuje dane do struktury dla silnikow

	char buffer[2];

	sprintf(buffer, "%c", enginesFrame[0]);		// Przepisanie z uint8_t na char (nie wiem czemu funkcja atoi nie chce konwertowac uint8_t)
	enginesData.multiplier = atoi(buffer);		// Konwersja i przypisanie wartosci z ramki
												// Ponizej dokladnie to samo dla reszty zmiennych
	sprintf(buffer, "%c", enginesFrame[1]);
	enginesData.leftPower = atoi(buffer);

	sprintf(buffer, "%c", enginesFrame[2]);
	enginesData.rightPower = atoi(buffer);

	sprintf(buffer, "%c", enginesFrame[3]);
	enginesData.leftDirection = atoi(buffer);

	sprintf(buffer, "%c", enginesFrame[4]);
	enginesData.rightDirection = atoi(buffer);
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart){	// Funkcja wykonywana, gdy USART odbierze dane i wywola przerwanie

	uint8_t wiadomosc[8] = "Odebrano";

	if (huart->Instance == USART2){		// Sprawdzenie czy wyslano przez USART1, nie trzeba tego uzywac dopoki uzywamy tylko jednego modulu USART
		HAL_UART_Transmit_IT(&huart2, wiadomosc, 8);	// Wyslanie potwierdzenia odebrania danych
		HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin); // Zmiana stanu pinu na diodzie LED
		convertEnginesFrame();		// Wywolanie funkcji adresuj�cej odebrane dane do struktury
		HAL_UART_Receive_IT(&huart2, enginesFrame, 5);
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
