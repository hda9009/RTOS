/*
 * 1. print_Hello.c
 *
 *  Created on: 03-Dec-2019
 *      Author: black_Pearl
 */

/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/
/*
 * WAP to create 2 task [task-1 & task-2 with same priorities.
 * Task-1 executes: it should print "hello from task-1"
 * task-2 executes: it should print "hello from task-2"
 *
 * Case-1: Use ARM Semi-hosting feature to print logs on the console
 * Case-2 Use UART peripheral of the MCU to pring logs
 *
 * use system clock of the MCU = 16 MHz
 */

#include <stm32f4xx.h>
#include <FreeRTOS.h>
#include <task.h>
#include<stdint.h>
#include<stdio.h>

TaskHandle_t xTaskHandle1 = NULL;
TaskHandle_t xTaskHandle2 = NULL;

// prototypes
void vTask1_Handler(void *param);
void vTask2_Handler(void *param);
static void prvSetupHardware(void);

// used for semi hosting
extern void initialise_monitor_handles();

int main(void){

	initialise_monitor_handles();
	printf("Hello World");


	// Reset the RCC clock configuration to the default reset state
	// 1. HSI ON, PLL OFF, SysCLK = 16 MHz, CPU CLK = 16 MHz
	RCC_DeInit();

	// 2. Update the SystemCoreClock variable
	SystemCoreClockUpdate();		// 16MHz
	// or SystemCoreClock = 16000000;

	prvSetupHardware();


	// 3. Create 2 tasks
	xTaskCreate(vTask1_Handler, "Task-1", 130, NULL, 2, &xTaskHandle2);
	xTaskCreate(vTask2_Handler, "Task-2", 130, NULL, 2, &xTaskHandle1);
	// Minimum stack size = 130 words = 130*4 = 520 bytes

	// 4. Start the scheduler
	vTaskStartScheduler();		// function will never return to next instruction.
	// task will be running continuously

	// for loop will never execute because of scheduler. It will give the control to the task handler
	for(;;);
}

void vTask1_Handler(void *param){
	while(1){
		printf("Hello World from task1 \n");
		taskYIELD();
	}
}

void vTask2_Handler(void *param){
	while(1){
		printf("Hello World from task2 \n");
		taskYIELD();
	}
}

static void prvSetupHardware(void){
	/* a private function in order to implement all hardware specification
	 * means peripheral initializations
	 */
}
