/*
 * 014. Binary_Sema_Mutexe.c
 *
 *  Created on: 07-Apr-2020
 *      Author: black_Pearl
 */

#include<stdint.h>
#include<stdio.h>
#include<string.h>

#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include <semphr.h>
#include <sys/sched.h>

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

// prototypes
void vTask1_Handler(void *param);
void vTask2_Handler(void *param);

static void prvSetupHardware(void);
static void prvSetupUART(void);
void printmsg(char *msg);

char user_msg[255] = {0};
uint8_t data1;
SemaphoreHandle_t BinarySemaphore;

int main(void){
	DWT->CTRL |= (1<<0);	// Enable CYCCNT in DWT_CTRL
	RCC_DeInit();
	SystemCoreClockUpdate();		// 16MHz
	prvSetupHardware();

	sprintf(user_msg, "Hello! this is starting of application \r\n");
	printmsg(user_msg);

	BinarySemaphore = xSemaphoreCreateBinary();
	if(BinarySemaphore != NULL){
		xTaskCreate(vTask1_Handler, "Task-1", 500, NULL, 5, &xTaskHandle1);
		xTaskCreate(vTask2_Handler, "Task-2", 500, NULL, 2, &xTaskHandle2);
		xSemaphoreGive(BinarySemaphore);
		vTaskStartScheduler();		// function will never return to next instruction.
	}else{
		 sprintf(user_msg,"binary semaphore creation failed\r\n");
		 printmsg(user_msg);
	 }
	pthread_mutex_t mutux = _PTHREAD_MUTEX_INITIALIZER;

	for(;;);
}

void vTask1_Handler(void *param){

	while(1){
		if(xSemaphoreTake(BinarySemaphore, 0) == pdTRUE){

			printmsg("hello! from task 1\r\n");
			xSemaphoreGive(BinarySemaphore);
	//		taskYIELD();
			vTaskDelay(1000);
		}
	}
}

void vTask2_Handler(void *param){
	while(1){
		if(xSemaphoreTake(BinarySemaphore, 0) == pdTRUE){
			printmsg("hello! from task 2\r\n");
			xSemaphoreGive(BinarySemaphore);
	//		taskYIELD();
			vTaskDelay(1000);
		}
	}
}

void USART2_IRQHandler(void){
	QueueHandle_t xStatus;
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(USART2->SR &(0x20)){
		data1 = (USART2->DR & 0xFF);
		//lets notify the command handling task
		if(data1 != '\0'){
			xSemaphoreTakeFromISR(BinarySemaphore, &xHigherPriorityTaskWoken);
			sprintf(user_msg,"/*****Hello!, from USART*****/.\r\n");
			printmsg(user_msg);
			xSemaphoreGiveFromISR(BinarySemaphore, &xHigherPriorityTaskWoken);
			}else{
				sprintf(user_msg,"Could not send to the queue.\r\n");
				printmsg(user_msg);
			}
		}
	if(xHigherPriorityTaskWoken){
		taskYIELD();
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
	GPIO_InitTypeDef gpio_uart_pins;
	USART_InitTypeDef uart2_init;

	//1. Enable the UART2  and GPIOA Peripheral clock
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA,ENABLE);

	//PA2 is UART2_TX, PA3 is UART2_RX

	//2. Alternate function configuration of MCU pins to behave as UART2 TX and RX

	//zeroing each and every member element of the structure
	memset(&gpio_uart_pins,0,sizeof(gpio_uart_pins));

	gpio_uart_pins.GPIO_Pin = GPIO_Pin_2 | GPIO_Pin_3;
	gpio_uart_pins.GPIO_Mode = GPIO_Mode_AF;
	gpio_uart_pins.GPIO_PuPd = GPIO_PuPd_UP;
	gpio_uart_pins.GPIO_OType= GPIO_OType_PP;
	gpio_uart_pins.GPIO_Speed = GPIO_High_Speed;
	GPIO_Init(GPIOA, &gpio_uart_pins);


	//3. AF mode settings for the pins
    GPIO_PinAFConfig(GPIOA,GPIO_PinSource2,GPIO_AF_USART2); //PA2
	GPIO_PinAFConfig(GPIOA,GPIO_PinSource3,GPIO_AF_USART2); //PA3

	//4. UART parameter initializations
	//zeroing each and every member element of the structure
	memset(&uart2_init,0,sizeof(uart2_init));

	uart2_init.USART_BaudRate = 9600;
	uart2_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart2_init.USART_Mode =  USART_Mode_Tx | USART_Mode_Rx;
	uart2_init.USART_Parity = USART_Parity_No;
	uart2_init.USART_StopBits = USART_StopBits_1;
	uart2_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2,&uart2_init);

	//lets enable the UART byte reception interrupt in the microcontroller
	USART_ITConfig(USART2,USART_IT_RXNE,ENABLE);

	//lets set the priority in NVIC for the UART2 interrupt
	NVIC_SetPriority(USART2_IRQn,5);

	//enable the UART2 IRQ in the NVIC
	NVIC_EnableIRQ(USART2_IRQn);

	//5. Enable the UART2 peripheral
	USART_Cmd(USART2,ENABLE);

}

