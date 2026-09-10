#include "stm32f411xe.h"
#include "stm32f4xx.h"
#include "system_stm32f4xx.h"
#include <stdint.h>

// port a
#define TX_pin 9
#define RX_pin 10 

void _enable_FPU(){
    SCB -> CPACR |= 0xF<<20;
}
uint32_t _round_to_int(float input){
    uint32_t output = input;

    if((input - output) <= 0.5f){
        output ++;
    }

    return output;
}

// initializes PA9 as TX and PA10 as RX
void uart1_init(uint32_t baud_rate, uint32_t APB2_clk){

    // -------------- GPIO ----------------
    RCC -> AHB1ENR |= RCC_AHB1ENR_GPIOAEN; 

    GPIOA -> MODER &= ~(GPIO_MODER_MODE9 | GPIO_MODER_MODE10);
    GPIOA -> MODER |= GPIO_MODER_MODE9_1 | GPIO_MODER_MODE10_1;

    GPIOA -> OSPEEDR &= ~(GPIO_OSPEEDER_OSPEEDR9 | GPIO_OSPEEDER_OSPEEDR10);
    GPIOA -> OSPEEDR |= GPIO_OSPEEDER_OSPEEDR9_1 | GPIO_OSPEEDER_OSPEEDR10_1;

    GPIOA -> AFR[1] &= ~(GPIO_AFRH_AFSEL9 | GPIO_AFRH_AFSEL10);
    GPIOA -> AFR[1] |= (7<<GPIO_AFRH_AFSEL9_Pos) | (7<<GPIO_AFRH_AFSEL10_Pos);

    // -------------- USART ----------------
    RCC -> APB2ENR |= RCC_APB2ENR_USART1EN;

    USART1 -> CR1 |= USART_CR1_UE; // UART enable
    USART1 -> CR1 &= ~ USART_CR1_M; // 8 data bits
    USART1 -> CR3 &= ~ USART_CR2_STOP_Msk; // One stop bit


    // baud rate 
    USART1 -> CR1 &= ~(USART_CR1_OVER8);
    _enable_FPU();

    float USARTDIV = (float)APB2_clk/(16 * baud_rate);
    uint16_t DIV_Mantissa = USARTDIV;
    float DIV_Fraction = 16*(USARTDIV - DIV_Mantissa);
    uint8_t rounded_DIV_Fraction = _round_to_int(DIV_Fraction);

    USART1 -> BRR &= ~(USART_BRR_DIV_Mantissa_Msk | USART_BRR_DIV_Fraction_Msk);
    USART1 -> BRR |= (DIV_Mantissa << USART_BRR_DIV_Mantissa_Pos)|(rounded_DIV_Fraction << USART_BRR_DIV_Fraction_Pos);
}

void uart1_transmit(uint8_t data[], uint64_t data_amount){

    USART1 -> CR1 |= USART_CR1_TE;

    for(uint64_t i = 0; i<data_amount; i++){

        while(!(USART1->SR & USART_SR_TXE)){} // wait for transmitter to empty

        USART1->DR |= data[i];
    }

    while(!(USART1->SR & USART_SR_TC)){} // wait for transmition to complete
}

void uart1_recieve(uint8_t data_storage[], uint64_t data_amount){

    USART1 -> CR1 |= USART_CR1_RE;

    for(uint64_t i = 0; i<data_amount; i++){

        while(!(USART1->SR & USART_SR_RXNE)){} // wait for data to get recieved

        data_storage[i] = USART1 -> DR;
    }
}