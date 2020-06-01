/*
 * LED_Btn_IT.c
 *
 *  Created on: 08-Mar-2020
 *      Author: black_Pearl
 */


#include<stm32f4xx.h>
#include<FreeRTOS.h>
#include<Task.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

void LED(void *param);
void BUTTON(void *param);
void Send_data(char *data);
char user_msg[50];
int flag = false;

int main(){
	DWT->CTRL |= (1<<0); 		// Enable DWT CYCCNT-->> This will help in capturing all the time related information in terminal window of segger
	RCC_DeInit();
	SystemCoreClockUpdate();

	RCC->APB1ENR |= 0x20000;	//(1<<17);		// USART2 ENABLE
	RCC->AHB1ENR |= 0x5;		//(101<<0);			// GPIOA & GPIOC Enable
	RCC->APB2ENR |= (1<<14);	// SYSConfig for PC13 Interrupt

	SYSCFG->EXTICR[3] = 0x20;
	EXTI->IMR = (1<<13);		// Unmasking 13 bit
	EXTI->RTSR = (1<<13);		// Rising triggering

	GPIOA->AFR[0] |= 0x0700;	// PA2 AF7
	GPIOA->MODER |= 0x0020;		// PA2 Set

	GPIOA->MODER |= (1<<10);	// PA5
	GPIOC->MODER = (00 << 26);	// PC13
	GPIOC->PUPDR = (2<< 26);

	USART2->BRR = 0x0683;		// 9600 @ 16 MHz
	USART2->CR1 = 0x0008;		// TE Enable
	USART2->CR1 |= 0x2000;		// UE Enable

	NVIC_EnableIRQ(EXTI15_10_IRQn);
	NVIC_SetPriority(EXTI15_10_IRQn, 5);


	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SEGGER_SYSVIEW_Conf();		// To start Segger Recording
	SEGGER_SYSVIEW_Start();
	xTaskCreate(LED, "LED", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();
}

void LED(void *param){
	while(1){
	if(flag == 1){
		GPIOA->ODR = (0<<5);
//			Send_data("Task1\n\r");
			taskYIELD();
		}
	else{
		GPIOA->ODR = (1<<5);
		taskYIELD();
	}
	}
}
void BUTTON(void *param){	// Settings
	flag ^= 1;
}

void Send_data(char *data){
	for(int i=0; i<strlen(data); i++){
		while(!(USART2->SR & 0x0080));
		USART2->DR = (data[i] & 0xFF);
	}
}

void EXTI15_10_IRQHandler(void){
	traceISR_ENTER();
	EXTI_ClearITPendingBit(EXTI_Line13);
	BUTTON(NULL);
	traceISR_EXIT();
}
