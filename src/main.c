#include<stm32f4xx.h>
#include<FreeRTOS.h>
#include<Task.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

void Task1(void *param);
void Task2(void *param);
void Send_data(char *data);
char user_msg[50];
int flag = false;

int main(){
	DWT->CTRL |= (1<<0); 		// Enable DWT CYCCNT-->> This will help in capturing all the time related information in terminal window of segger
	RCC_DeInit();
	SystemCoreClockUpdate();
	RCC->APB1ENR |= 0x20000;	//(1<<17);		// USART2 ENABLE
	RCC->AHB1ENR |= 1;				//(1<<0);			// GPIOA Enable
	GPIOA->AFR[0] |= 0x0700;	// PA2 AF7
	GPIOA->MODER |= 0x0020;		// PA2 Set

	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
	USART2->CR1 = 0x0008;			// TE Enable
	USART2->CR1 |= 0x2000;		// UE Enable
	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SEGGER_SYSVIEW_Conf();		// To start Segger Recording
	SEGGER_SYSVIEW_Start();
	xTaskCreate(Task1,"Task-1", 130, NULL, 0, NULL);
	xTaskCreate(Task2,"Task-2", 130, NULL, 0, NULL);
	vTaskStartScheduler();
}

void Task1(void *param){
		while(1){
	//if(flag == false){
			Send_data("Task1\n\r");
		//	flag = true;
			taskYIELD();
		}
	//}
}
void Task2(void *param){	// Settings
		while(1){
//	if(flag == true){
			Send_data("Task2\n\r");
		//	flag = false;
			taskYIELD();
		//}
	}
}

void Send_data(char *data){
	for(int i=0; i<strlen(data); i++){
		while(!(USART2->SR & 0x0080));
		USART2->DR = (data[i] & 0xFF);
	}
}
