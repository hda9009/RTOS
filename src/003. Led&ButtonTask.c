/*
 * Create 2 task in your FreeRTOS application led_task & button_task
 * Button Task should continuously poll the button status of the borad and it pressed
 * it should update the flag variable.
 * Led Task should turn on the LED if button flag is SET, otherwise it should turn of the LED.
 * Use same FreeRTOS task priorities for both the tasks.
 *
 */

#include<stm32f4xx.h>
#include<freeRTOS.h>
#include<task.h>
#include<string.h>
#include<stdint.h>


static void prvSetupHardware(void);
static void prvSetupUART(void);
//void printmsg(char *msg);
static void prvSetupGPIO();

void led_task_handler(void *param);
void button_task_handler(void *param);

#define TRUE				1
#define FALSE				0
int flag = TRUE;

int main(void){

	RCC_DeInit();
	SystemCoreClockUpdate();		// 16MHz
	prvSetupHardware();

	SEGGER_SYSVIEW_Conf();
	SEGGER_SYSVIEW_Start();

	xTaskCreate(led_task_handler, "Led", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	xTaskCreate(button_task_handler, "Button", configMINIMAL_STACK_SIZE, NULL, 1, NULL);
	vTaskStartScheduler();
}

void led_task_handler(void *param){
	while(1){
		if(flag ==1){
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, 1 );
		}
		else{
			GPIO_WriteBit(GPIOA, GPIO_Pin_5, 0 );
		}
	}
}

void button_task_handler(void *param){
	while(1){
		if(GPIO_ReadInputDataBit(GPIOC,GPIO_Pin_13)){
			flag = FALSE;
		}
		else{
			flag = TRUE;
		}
	}
}

static void prvSetupHardware(void){
	prvSetupGPIO();
	prvSetupUART();

}

//void printmsg(char *msg){
//	for(uint32_t i=0; i < strlen(msg); i++){
//		while(USART_GetFlagStatus(USART2, USART_FLAG_TXE) != SET){;}
//		USART_SendData(USART2, msg[i]);
//	}
//		while ( USART_GetFlagStatus(USART2,USART_FLAG_TC) != SET){;}
//}

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

	uart2_init.USART_BaudRate = 115200;
	uart2_init.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	uart2_init.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	uart2_init.USART_Parity = USART_Parity_No;
	uart2_init.USART_StopBits = USART_StopBits_1;
	uart2_init.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART2, &uart2_init);

	// 5. Enable the UART
	USART_Cmd(USART2, ENABLE);		// UE Enabling
}

static void prvSetupGPIO(){
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA, ENABLE);
		RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);
		GPIO_InitTypeDef led;
		led.GPIO_Pin = GPIO_Pin_5;
		led.GPIO_Mode= GPIO_Mode_OUT;
		led.GPIO_OType = GPIO_OType_PP;
		led.GPIO_PuPd = GPIO_PuPd_NOPULL;
		led.GPIO_Speed = GPIO_Fast_Speed;
		GPIO_Init(GPIOA, &led);

		GPIO_InitTypeDef button;
		button.GPIO_Pin = GPIO_Pin_13;
		button.GPIO_Mode= GPIO_Mode_IN;
		GPIO_Init(GPIOC, &button);
}
