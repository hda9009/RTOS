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
uint8_t Flag;

void Task_1(void *param);
void Task_2(void *param);
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
	RCC->APB2ENR = (1<<14);		// Sysconfig Enable

	GPIOA->AFR[0] |= 0x0700;	// PA2 AF7
	GPIOA->MODER |= 0x0020;		// PA2 Set

	GPIOA->MODER |= (1<<10);	// PA5
	GPIOA->MODER |= (1<<8);		// PA4
	GPIOA->MODER |= 0x05;		// PA0 PA1
	GPIOC->MODER = (00 << 26);
	GPIOC->PUPDR = (2<< 26);

	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
	USART2->CR1 = 0x0008;			// TE Enable
	USART2->CR1 |= 0x2000;			// UE Enable
	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SYSCFG->EXTICR[3] = 0x20;		// PC13
	EXTI->IMR = (1<<13);			// Button Interrupt
	EXTI->FTSR = (1<<13);
	NVIC_SetPriority(EXTI15_10_IRQn, 5);	// Highest priority
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	SEGGER_SYSVIEW_Conf();			// To start Segger Recording
	SEGGER_SYSVIEW_Start();
	xTaskCreate(Task_1, "Task_1", 500, NULL, 3, &xTaskHandle1);
	xTaskCreate(Task_2, "Task_2", 500, NULL, 2, &xTaskHandle2);	// Button task has higher priority than Led Task
	vTaskStartScheduler();

	while(1){

	}
}

void Task_1(void *param){
	while(1){

			sprintf(user_msg, "\n\rCurrent Status: %ld \r\n", ((GPIOA->IDR >> 5) && 0x01));
			Send_data(user_msg);

			GPIOA->ODR ^= (1<<5);
			vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

void Task_2(void *param){
	uint8_t p1, p2;
	sprintf(user_msg, "\n\rTask-2 is running\r\n");
	Send_data(user_msg);

	sprintf(user_msg, "Task-1 Priority: %ld \r\n", uxTaskPriorityGet(xTaskHandle1));
	Send_data(user_msg);

	sprintf(user_msg, "Task-2 Priority: %ld \r\n", uxTaskPriorityGet(xTaskHandle2));
	Send_data(user_msg);

	while(1){
		if(Flag == true){
			sprintf(user_msg, "\r\nTask-2 is running\r\n");
			Send_data(user_msg);

			sprintf(user_msg, "Task-1 Priority: %ld \r\n", uxTaskPriorityGet(xTaskHandle1));
			Send_data(user_msg);

			sprintf(user_msg, "Task-2 Priority: %ld \r\n", uxTaskPriorityGet(xTaskHandle2));
			Send_data(user_msg);

			Flag = false;
			p1 = uxTaskPriorityGet(xTaskHandle1);
			p2 = uxTaskPriorityGet(xTaskHandle2);

			vTaskPrioritySet(xTaskHandle1, p2);
			vTaskPrioritySet(xTaskHandle2, p1);
		}
		else{
			GPIOA->ODR ^= (1<<5);
			rtos_delay(1000);
		}
	}
}

void EXTI15_10_IRQHandler(void){
	traceISR_ENTER();
	EXTI->PR = (1<<13);
	Flag = true;
	traceISR_EXIT();
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


