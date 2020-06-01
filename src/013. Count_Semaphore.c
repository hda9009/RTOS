/*
 * 012. Binary_Semaphore_ISR.c
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
void vHandlerTask(void *param);
void vPriorityTask(void *param);

char user_msg[100];
int flag = false;
uint8_t data1 = '\0';
unsigned int xWorkTicketID;

SemaphoreHandle_t xCountSemaphore;
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


	sprintf(user_msg, "Hello! this is starting of application \r\n");
	Send_data(user_msg);

	SYSCFG->EXTICR[3] = 0x20;		// PC13
	EXTI->IMR = (1<<13);			// Button Interrupt
	EXTI->FTSR = (1<<13);
	NVIC_SetPriority(EXTI15_10_IRQn, 5);	// Highest priority
	NVIC_EnableIRQ(EXTI15_10_IRQn);

	SEGGER_SYSVIEW_Conf();			// To start Segger Recording
	SEGGER_SYSVIEW_Start();

	xCountSemaphore = xSemaphoreCreateCounting(10, 0);

	if((xCountSemaphore != NULL)){
		xTaskCreate(vHandlerTask, "Handler_Task", 500, NULL, 1, NULL);
		xTaskCreate(vPriorityTask, "Priority_Task", 500, NULL, 3, NULL);
		vTaskStartScheduler();
	}else{
		sprintf(user_msg, "Queue/ Semaphore Created Failed..\n\r");
		Send_data(user_msg);
	}
	while(1){

	}
	return 0;
}


void vHandlerTask(void *param){
	static uint8_t i;
	while(1){
		xSemaphoreTake( xCountSemaphore, portMAX_DELAY );
		sprintf(user_msg, "%d Counts.\r\n", ++i);
		Send_data(user_msg);
		if(i==5){
			i=0;
		}
//		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}


void vPriorityTask(void *param){
	while(1){
		sprintf(user_msg, "Periodic task - Pending the interrupt.\r\n" );
		Send_data(user_msg);
		vTaskDelay(pdMS_TO_TICKS(2000));

		//pend the interrupt
//		NVIC_SetPendingIRQ(EXTI15_10_IRQn);

//		sprintf(user_msg, "Periodic task - Resuming.\r\n" );
//		Send_data(user_msg);
	}
}

void EXTI15_10_IRQHandler(void){
	EXTI->PR = (1<<13);
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	sprintf(user_msg,"\n\r/************GPIO_Button_Handler*********************/\r\n");
	Send_data(user_msg);

	xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
//	xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
//	xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
//	xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
//	xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
	portEND_SWITCHING_ISR( xHigherPriorityTaskWoken);
	/*
	portEND_SWITCHING_ISR() will force a context switch to the newly unblocked
    higher priority task.
    NOTE: The syntax for forcing a context switch within an ISR varies between
    FreeRTOS ports.  The portEND_SWITCHING_ISR() macro is provided as part of
    the Cortex M3 port layer for this purpose.  taskYIELD() must never be called
    from an ISR!
	 */
}

void USART2_IRQHandler(void){
	BaseType_t xHigherPriorityTaskWoken = pdFALSE;
	if(USART2->SR &(0x20)){
		data1 = (USART2->DR & 0xFF);
		//lets notify the command handling task
		if(data1 != '\0'){
				sprintf(user_msg,"\n\r/************USART_Handler*********************/\r\n");
				Send_data(user_msg);
				xSemaphoreGiveFromISR( xCountSemaphore, &xHigherPriorityTaskWoken );
				portEND_SWITCHING_ISR( xHigherPriorityTaskWoken);
		}
	}
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


