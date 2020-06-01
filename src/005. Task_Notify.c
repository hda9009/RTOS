/*
 * 005. Task_Notify.c
 *
 *  Created on: 10-Mar-2020
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

	GPIOA->MODER |= (1<<10);
	GPIOC->MODER = (00 << 26);
	GPIOC->PUPDR = (2<< 26);

	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
	USART2->CR1 = 0x0008;			// TE Enable
	USART2->CR1 |= 0x2000;		// UE Enable
	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SEGGER_SYSVIEW_Conf();		// To start Segger Recording
	SEGGER_SYSVIEW_Start();
	xTaskCreate(LED, "LED", 500, NULL, 2, &xTaskHandle1);
	xTaskCreate(BUTTON, "BUTTON", 500, NULL, 2, &xTaskHandle2);
	vTaskStartScheduler();
}

void LED(void *param){
	while(1){
		uint32_t count=0;
		// wait until led_task receives any notification
//		if(xTaskNotifyWait(0, 0, 0 , portMAX_DELAY) == pdTRUE){	// portMAX_DELAY: 0xFFFFFFFF
		if(xTaskNotifyWait(0, 0, &count , portMAX_DELAY) == pdPASS){
			GPIOA->ODR ^= (1<<5);
			sprintf(user_msg, "Count: % ld\n\r", count);
			Send_data(user_msg);
			taskYIELD();
		}
	}
}
void BUTTON(void *param){	// Settings
		while(1){
			if((GPIOC->IDR & (1<<13)) == 0){
				//
				rtos_delay(500);
				// send the notification to led_task
//				xTaskNotify(xTaskHandle1, NULL, eNoAction);
				Send_data("Testing the Notification API\n\r");
				xTaskNotify(xTaskHandle1, NULL, eIncrement);
				taskYIELD();
			}
			else{
				flag = 1;
				taskYIELD();
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
	while(xTaskGetTickCount() <= (current_tick_count + delay_in_ticks));
}
