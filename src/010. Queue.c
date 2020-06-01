/*
 * 006. Task_Delete.c
 *
 *  Created on: 16-Mar-2020
 *      Author: black_Pearl
 */

#include<stm32f4xx.h>
#include<stdint.h>
#include<stdio.h>
#include<string.h>
#include<stdbool.h>

#include<FreeRTOS.h>
#include<Task.h>
#include<queue.h>
#include<Timers.h>

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;
TaskHandle_t xTaskHandle3 = NULL;
TaskHandle_t xTaskHandle4 = NULL;
uint8_t Flag;

void Send_data(char *data);
void rtos_delay(uint32_t delay_in_ms);
uint8_t getcommandCode(uint8_t *buffer);
uint8_t getArguments(uint8_t *buffer);


// Tasks Handle
void vTask1_menu_display(void *param);
void vTask2_cmd_handling(void *param);
void vTask3_cmd_processing(void *param);
void vTask4_uart_write(void *param);

// Queue Handle
QueueHandle_t command_queue = NULL;
QueueHandle_t uart_write_queue = NULL;

char user_msg[100];
int flag = false;

uint8_t data[10];
int data1;
uint8_t number;

// command structure
typedef struct APP_CMD{
	uint8_t COMMAND_NUM;
	uint8_t COMMAND_ARGS[10];
}APP_CMD_t;

// Menu
char menu[] = { "\
		\r\nLED_ON				---->>> 1 \
		\r\nLED_OFF				---->>> 2 \
		\r\nLED_Toggle			---->>> 3 \
		\r\nLED_Toggle_OFF		---->>> 4 \
		\r\nLED_Read_STATUS		---->>> 5 \
		\r\nRTC_Print_DateTime	---->>> 6 \
		\r\nType your own Option: 		"};

#define LED_ON				1
#define LED_OFF				2
#define LED_Toggle			3
#define LED_Toggle_OFF		4
#define LED_Read_STATUS		5
#define RTC_Print_DateTime	6

int main(){
	DWT->CTRL |= (1<<0); 		// Enable DWT CYCCNT-->> This will help in capturing all the time related information in terminal window of segger
	RCC_DeInit();
	SystemCoreClockUpdate();

	RCC->APB1ENR |= 0x20000;	//(1<<17);		// USART2 ENABLE
	RCC->AHB1ENR |= 0x5;		//(101<<0);			// GPIOA & GPIOC Enable
	RCC->APB2ENR = (1<<14);		// Sysconfig Enable

	GPIOA->AFR[0] |= 0x7700;	// PA2 PA3 AF7
	GPIOA->MODER |= 0x00A0;		// PA2 Set(TX) and PA3(RX)

	GPIOA->MODER |= (1<<10);	// PA5
	GPIOA->MODER |= (1<<8);		// PA4
	GPIOA->MODER |= 0x05;		// PA0 PA1
	GPIOC->MODER = (00 << 26);
	GPIOC->PUPDR = (2<< 26);

	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
	USART2->CR1 = 0x202C;			// RXNE and TE, RE Enable
//	USART2->CR1 |= 0x2000;			// UE Enable
//	NVIC_SetPriority(USART2_IRQn, 5);	// Highest priority
	NVIC_EnableIRQ(USART2_IRQn);

//	sprintf(user_msg, "Hello! this is starting of application \r\n");
//	Send_data(user_msg);

//	SYSCFG->EXTICR[3] = 0x20;		// PC13
//	EXTI->IMR = (1<<13);			// Button Interrupt
//	EXTI->FTSR = (1<<13);
//	NVIC_SetPriority(EXTI15_10_IRQn, 5);	// Highest priority
//	NVIC_EnableIRQ(EXTI15_10_IRQn);

	SEGGER_SYSVIEW_Conf();			// To start Segger Recording
	SEGGER_SYSVIEW_Start();

	// Queue Creation
	command_queue = xQueueCreate(10, sizeof(APP_CMD_t *));
	uart_write_queue = xQueueCreate(10, sizeof(char *));

	if((command_queue != NULL) && (uart_write_queue != NULL)){
		// creating Tasks
		xTaskCreate(vTask1_menu_display, "Menu_Display", 500, NULL, 1, &xTaskHandle1);
		xTaskCreate(vTask2_cmd_handling, "CMD_Handling", 500, NULL, 1, &xTaskHandle2);
		xTaskCreate(vTask3_cmd_processing, "CMD_Processing", 500, NULL, 1, &xTaskHandle3);
		xTaskCreate(vTask4_uart_write, "UART_Write", 500, NULL, 1, &xTaskHandle4);
		vTaskStartScheduler();
	}
	else{
		sprintf(user_msg, "Queue Creation Failed! Sorry for the Inconvenience \r\n");
		Send_data(user_msg);
	}

	while(1){

	}
	return 0;
}

void vTask1_menu_display(void *param){
	char *pData = menu;		// pData = &menu[0]
	while(1){
		xQueueSend(uart_write_queue, &pData, portMAX_DELAY);
// it send the data to the queue and gets blocks there until someone notifies it
		// wait until someone notifies
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);		// wait until it receives notification
	}
}
uint8_t command_code;
void vTask2_cmd_handling(void *param){
	APP_CMD_t *new_cmd;			// structure
	while(1){
		xTaskNotifyWait(0, 0, NULL, portMAX_DELAY);
//		command_code = getcommandCode(data[0]);
		command_code = (data[0]);
		new_cmd = (APP_CMD_t *) pvPortMalloc(sizeof(APP_CMD_t));	// allocating structure memory dynamically
		new_cmd->COMMAND_NUM = command_code;
		getArguments(new_cmd->COMMAND_ARGS);

		xQueueSend(command_queue, &new_cmd, portMAX_DELAY);
	}
}
void vTask3_cmd_processing(void *param){
	APP_CMD_t *new_cmd;
	char task_msg[50];
	while(1){
		xQueueReceive(command_queue, (void*)&new_cmd, portMAX_DELAY);
		if(new_cmd->COMMAND_NUM == LED_ON){
			GPIOA->ODR = (1<<5);
		}else if(new_cmd->COMMAND_NUM == LED_OFF){
			GPIOA->ODR = (0<<5);
		}else if(new_cmd->COMMAND_NUM == LED_Toggle){
			GPIOA->ODR ^= (1<<5);
		}else if(new_cmd->COMMAND_NUM == LED_Toggle_OFF){
			GPIOA->ODR = (0<<5);
		}else if(new_cmd->COMMAND_NUM == LED_Read_STATUS){
//			GPIO_ReadOutputDataBit(GPIOA, GPIO_PinSource5);
			sprintf(task_msg, "\n\rLed Status is: %d \n\r", GPIO_ReadOutputDataBit(GPIOA, GPIO_PinSource5));
			xQueueSend(uart_write_queue, &task_msg, portMAX_DELAY);
		}else if(new_cmd->COMMAND_NUM == RTC_Print_DateTime){

		}else{
			// print error message
			sprintf(task_msg, "\n\rYou have Entered an Invalid Command \n\r");
			xQueueSend(uart_write_queue, &task_msg, portMAX_DELAY);
		}
	}
}
void vTask4_uart_write(void *param){
	char *pData = NULL;
	while(1){
		xQueueReceive(uart_write_queue, &pData, portMAX_DELAY);
		Send_data(pData);
	}
}

uint8_t getcommandCode(uint8_t *buffer){
	return buffer[0] - 48;
}

uint8_t getArguments(uint8_t *buffer){

}

void EXTI15_10_IRQHandler(void){
	traceISR_ENTER();
	EXTI->PR = (1<<13);
	Flag = true;
	traceISR_EXIT();
}

void USART2_IRQHandler(void){
	uint16_t data_byte;
		BaseType_t xHigherPriorityTaskWoken = pdFALSE;

		if( USART_GetFlagStatus(USART2,USART_FLAG_RXNE) )
		{
			//a data byte is received from the user
			data_byte = USART_ReceiveData(USART2);

			data1 = (data_byte & 0xFF) ;

			if(data_byte == '\r')
			{
				//then user is finished entering the data

				//reset the command_len variable
//				command_len = 0;

				//lets notify the command handling task
				xTaskNotifyFromISR(xTaskHandle2,0,eNoAction,&xHigherPriorityTaskWoken);

				xTaskNotifyFromISR(xTaskHandle1,0,eNoAction,&xHigherPriorityTaskWoken);
			}

		}

		// if the above freertos apis wake up any higher priority task, then yield the processor to the
		//higher priority task which is just woken up.

		if(xHigherPriorityTaskWoken)
		{
			taskYIELD();
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



