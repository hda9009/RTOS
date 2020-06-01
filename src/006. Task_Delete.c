/*
 * 006. Task_Delete.c
 *
 *  Created on: 16-Mar-2020
 *      Author: black_Pearl
 */

#include<stm32f4xx.h>
#include<FreeRTOS.h>
#include<Task.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

void LED(void *param);
void BUTTON(void *param);
void Send_data(char *data);
char user_msg[100];
int flag = false;

void rtos_delay(uint32_t delay_in_ms);

int main(){
	DWT->CTRL |= (1<<0); 		// Enable DWT CYCCNT-->> This will help in capturing all the time related information in terminal window of segger
	RCC_DeInit();
	SystemCoreClockUpdate();
	RCC->APB1ENR |= 0x20000;	//(1<<17);		// USART2 ENABLE
	RCC->AHB1ENR |= 0x5;		//(101<<0);			// GPIOA & GPIOC Enable

	GPIOA->AFR[0] |= 0x0700;	// PA2 AF7
	GPIOA->MODER |= 0x0020;		// PA2 Set

	GPIOA->MODER |= (1<<10);	// PA5
	GPIOA->MODER |= (1<<8);		// PA4
	GPIOA->MODER |= 0x05;		// PA0 PA1
	GPIOC->MODER = (00 << 26);
	GPIOC->PUPDR = (2<< 26);

	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
	USART2->CR1 = 0x0008;			// TE Enable
	USART2->CR1 |= 0x2000;		// UE Enable
	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SEGGER_SYSVIEW_Conf();		// To start Segger Recording
	SEGGER_SYSVIEW_Start();
	xTaskCreate(LED, "LED", 500, NULL, 1, &xTaskHandle1);
	xTaskCreate(BUTTON, "BUTTON", 500, NULL, 2, &xTaskHandle2);	// Button task has higher priority than Led Task
	vTaskStartScheduler();
	while(1){
		GPIOA->ODR ^= (1<<5);
		rtos_delay(1000);

	}
}

void LED(void *param){
//	uint32_t count=0;
	while(1){
	sprintf(user_msg, "\nLed Task Running\n\r");
		sprintf(user_msg, "LED Task-1\n\r");
		Send_data(user_msg);
//		rtos_delay(500);
		GPIOA->ODR = (1<<1);
		vTaskDelay(2000);
		GPIOA->ODR = (0<<1);
		vTaskDelay(2000);
//			taskYIELD();
		if((GPIOC->IDR & (1<<13)) == 0){
			sprintf(user_msg, "\n\rDelete Task-1\n\r");
			Send_data(user_msg);
			vTaskDelete(NULL);
//			taskYIELD();
		}
	}
}

void BUTTON(void *param){
	while(1){
	sprintf(user_msg, "\n\rButton Task Running\n\r");
		if((GPIOC->IDR & (1<<13)) == 0){
			sprintf(user_msg, "\n\rDelete Task-2\n\r");
			Send_data(user_msg);
			rtos_delay(500);
			vTaskDelete(NULL);
//			taskYIELD();
		}
		else{
//			sprintf(user_msg, "Task-2\n\r");
			sprintf(user_msg, "Button Task-2\n\r");
			Send_data(user_msg);
			Send_data(user_msg);
			GPIOA->ODR = (1<<0);
//			rtos_delay(500);
			vTaskDelay(1000);
//			GPIOA->ODR = (0<<0);
//			taskYIELD();
		}
	}
}

void Send_data(char *data){
	for(int i=0; i<strlen(data); i++){
		while(!(USART2->SR & 0x0080));
		USART2->DR = (data[i] & 0xFF);
	}
}

void rtos_delay(uint32_t delay_in_ms){
	// 1 tick count timings will be 1 mseconds
	uint32_t current_tick_count = xTaskGetTickCount();
	uint32_t delay_in_ticks = ((delay_in_ms * configTICK_RATE_HZ)/ 1000);	// conversion of time delay in count ticks
	while(xTaskGetTickCount() < (current_tick_count + delay_in_ticks));
}

