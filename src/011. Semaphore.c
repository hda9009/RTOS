/*
 * 011. Semaphore.c
 *
 *  Created on: 06-Apr-2020
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
#include<semphr.h>

uint8_t Flag;

void Send_data(char *data);
void rtos_delay(uint32_t delay_in_ms);
static void prvSetupUart(void);
void vManagerTask(void *param);
void vEmployeeTask(void *param);

char user_msg[100];
int flag = false;
uint8_t data1;
unsigned int xWorkTicketID;

SemaphoreHandle_t xWork;
QueueHandle_t xWorkQueue;

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

	prvSetupUart();

//	USART2->BRR = 0x0683;			// 9600 @ 16 MHz
//	USART2->CR1 = 0x202C;			// RXNE and TE, RE Enable
//	USART2->CR1 |= 0x2000;			// UE Enable
//	NVIC_SetPriority(USART2_IRQn, 5);	// Highest priority
//	NVIC_EnableIRQ(USART2_IRQn);

	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SYSCFG->EXTICR[3] = 0x20;		// PC13
	EXTI->IMR = (1<<13);			// Button Interrupt
	EXTI->FTSR = (1<<13);
	NVIC_SetPriority(EXTI15_10_IRQn, 5);	// Highest priority
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	SEGGER_SYSVIEW_Conf();			// To start Segger Recording
	SEGGER_SYSVIEW_Start();

	vSemaphoreCreateBinary(xWork);
	xWorkQueue = xQueueCreate(1, sizeof(unsigned int));

	if((xWorkQueue != NULL) && (xWork != NULL)){
		xTaskCreate(vManagerTask, "Manager_Task", 500, NULL, 3, NULL);
		xTaskCreate(vEmployeeTask, "Employee_Task", 500, NULL, 1, NULL);
		vTaskStartScheduler();
	}else{
		sprintf(user_msg, "Queue/ Semaphore Created Failed..\n\r");
		Send_data(user_msg);
	}
	while(1){

	}
	return 0;
}

void vManagerTask(void *param){
//	xSemaphoreGive(xWork);
//	portBASE_TYPE xStatus;
	QueueHandle_t xStatus;
	while(1){
		xWorkTicketID = (rand() & 0xFF);
		xStatus = xQueueSend(xWorkQueue, &xWorkTicketID, portMAX_DELAY);
		if(xStatus == pdPASS){
			xSemaphoreGive(xWork);
			taskYIELD();
		}else{
			sprintf(user_msg,"Could not send to the queue.\r\n");
			Send_data(user_msg);
		}
	}
}

void vEmployeeTask(void *param){
	portBASE_TYPE xStatus;
	while(1){
		xSemaphoreTake(xWork, 0);
		xStatus = xQueueReceive(xWorkQueue,&xWorkTicketID,0);
		if(xStatus == pdPASS){
			sprintf(user_msg,"Employee task : Working on Ticked id : %ld\r\n",xWorkTicketID);
			Send_data(user_msg);
			vTaskDelay(xWorkTicketID);
		}

	}
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
	if(USART2->SR &(0x20)){
		data1 = (USART2->DR & 0xFF);
		//lets notify the command handling task

	}


// if the above freertos apis wake up any higher priority task, then yield the processor to the
//higher priority task which is just woken up.

	if(xHigherPriorityTaskWoken){
		taskYIELD();
	}
}

void vApplicationIdleHook(void){
	__WFI();
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

static void prvSetupUart(void)
{
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


