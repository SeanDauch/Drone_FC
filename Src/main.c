#include "stm32f4xx.h"
#include <stdint.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "FreeRTOSConfig.h"
#include "task.h"

#include "debugging.h"

#define SystemClock 16000000
#define swo_baud 2000000
#define CRSF_baud 420000

void enable_FPU(){
    SCB->CPACR |= 0xF<<20;
}

/*
void vAssertCalled(void) {
    taskDISABLE_INTERRUPTS();
    for(;;);
}

void HardFault_Handler(void) {
    __asm("BKPT #0");
    while(1);
}*/

void gpio_pa11_pa12_init(void) {

    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIOA->MODER &= ~(GPIO_MODER_MODE11_Msk | GPIO_MODER_MODE12_Msk); 
    GPIOA->MODER |=  (1U << GPIO_MODER_MODE11_Pos) | (1U << GPIO_MODER_MODE12_Pos);                  
}


void vApplicationStackOverflowHook(TaskHandle_t xTask, char * pcTaskName){
    taskDISABLE_INTERRUPTS();
    while(1){}
}

void led_blink_11(){
    while(1){
        GPIOA->ODR ^= GPIO_ODR_OD11;

        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void led_blink_12(){
    while(1){
        GPIOA->ODR ^= GPIO_ODR_OD12;

        vTaskDelay(pdMS_TO_TICKS(333));
    }
}

#define reciever_length rc_frame_len
int main(){

    enable_FPU();
    gpio_pa11_pa12_init();

    BaseType_t status1 = xTaskCreate(led_blink_11, "blink 11", 128, NULL, 1, NULL);
    BaseType_t status2 = xTaskCreate(led_blink_12, "blink 12", 128, NULL, 2, NULL);
    (void)status1;
    (void)status2;

    vTaskStartScheduler();

    while(1){}
    return 1;
}