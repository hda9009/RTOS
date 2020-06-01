/*
 * 2. UART_Communication.c
 *
 *  Created on: 03-Dec-2019
 *      Author: black_Pearl
 */


/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/
/*
 * WAP to create 2 task [task-1 & task-2 with same priorities.
 * Task-1 executes: it should print "hello from task-1"
 * task-2 executes: it should print "hello from task-2"
 *
 * Case-1: Use ARM Semi-hosting feature to print logs on the console
 * Case-2 Use UART peripheral of the MCU to pring logs
 *
 * use system clock of the MCU = 16 MHz
 */

#include<stdint.h>
#include<stdio.h>
#include<string.h>

#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

// prototypes
void vTask1_Handler(void *param);
void vTask2_Handler(void *param);

static void prvSetupHardware(void);
static void prvSetupUART(void);
void printmsg(char *msg);

char user_msg[255] = {0};
//#define TRUE	1
//#define FALSE	0
//#define AVAILABLE		TRUE
//#define NOT_AVAILABLE	FALSE
//uint8_t UART_ACCESS_KEY = AVAILABLE;

// used for semi hosting
#ifdef USE_SEMIHOSTING
extern void initialise_monitor_handles();
#endif




int main(void){
	DWT->CTRL |= (1<<0);	// Enable CYCCNT in DWT_CTRL

#ifdef USE_SEMIHOSTING
	initialise_monitor_handles();
	printf("Hello World");
#endif

	// Reset the RCC clock configuration to the default reset state
	// 1. HSI ON, PLL OFF, SysCLK = 16 MHz, CPU CLK = 16 MHz
	RCC_DeInit();

	// 2. Update the SystemCoreClock variable
	SystemCoreClockUpdate();		// 16MHz
	prvSetupHardware();

	sprintf(user_msg, "Hello! this is starting of application \r\n");
	printmsg(user_msg);


	//Start Recording
//	SEGGER_SYSVIEW_Conf();
//	SEGGER_SYSVIEW_Start();

	// 3. Create 2 tasks
	xTaskCreate(vTask1_Handler, "Task-1", 130, NULL, 2, &xTaskHandle1);
	xTaskCreate(vTask2_Handler, "Task-2", 130, NULL, 2, &xTaskHandle2);
	// Minimum stack size = 130 words = 130*4 = 520 bytes

	// 4. Start the scheduler
	vTaskStartScheduler();		// function will never return to next instruction.
	// task will be running continuously

	for(;;);
}

void vTask1_Handler(void *param){
	while(1){
		//if(UART_ACCESS_KEY == AVAILABLE){
			printmsg("hello! from task 1\r\n");
			//UART_ACCESS_KEY = NOT_AVAILABLE;
			taskYIELD();
		//}
	}
}

void vTask2_Handler(void *param){
	while(1){
		//if(UART_ACCESS_KEY == NOT_AVAILABLE){
			printmsg("hello! from task 2\r\n");
			//UART_ACCESS_KEY = AVAILABLE;
			taskYIELD();
		//}
	}
}

static void prvSetupHardware(void){
	prvSetupUART();

}

void printmsg(char *msg){
	for(uint32_t i=0; i < strlen(msg); i++){
		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET){;}
		USART_SendData(USART2, msg[i]);
	}
		while ( USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET){;}
}

static void prvSetupUART(void){
/* a private function in order to implement all hardware specification
	 * means peripheral initializations
	 */

	// Initialization
	GPIO_InitTypeDef gpio_uart_pins;
	USART_InitTypeDef uart2_init;

	// 1. Enable UART2 Peripheral Clock: shown in nucleo user manual 6.8
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
	// PA2: TX and PA3:RX


	// 2. ALT Fun config. of MCU pins to behave as UART2 TX and RX
	memset(&gpio_uart_pins, 0, sizeof(gpio_uart_pins));

	gpio_uart_pins.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio_uart_pins.GPIO_Mode = GPIO_Mode_AF;	// For uart tx
	gpio_uart_pins.GPIO_PuPd = GPIO_PuPd_UP;		// Idle line of UART = 1
	gpio_uart_pins.GPIO_OType= GPIO_OType_PP;
	gpio_uart_pins.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOA, &gpio_uart_pins);

	// 3. AF mode Settings for the pin
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource2, GPIO_AF_USART2); // PA2 as UART_TX
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource3, GPIO_AF_USART2); // PA3 as UART_RX

	// 4. UART Parameter Initialization
	memset(&uart2_init, 0, sizeof(uart2_init));  // zeroing each and every member element of the structure

	uart2_init.USART_BaudRate = 9600;
	uart2_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart2_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	uart2_init.USART_Parity = USART_Parity_No;
	uart2_init.USART_StopBits = USART_StopBits_1;
	uart2_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &uart2_init);

	// 5. Enable the UART
	USART_Cmd(USART2, ENABLE);		// UE Enabling
}
