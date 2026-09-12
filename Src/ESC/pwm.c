#include "stm32f4xx.h"
#include <stdint.h>

#include "pwm.h"

// uses TIM1 and portA 8-11 
// (higher ARR means higher sens, but lower frequency)
void pwm_init(uint16_t ARR_value){

    // ------------ turn on clocks --------------
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;

    // ------------ config GPIO ------------
    GPIOA->MODER &= ~(GPIO_MODER_MODE8|GPIO_MODER_MODE9|GPIO_MODER_MODE10|GPIO_MODER_MODE11);
    GPIOA->MODER |= GPIO_MODER_MODE8_1|GPIO_MODER_MODE9_1|GPIO_MODER_MODE10_1|GPIO_MODER_MODE11_1; //alt

    GPIOA->OSPEEDR |= GPIO_OSPEEDR_OSPEED8_Msk|GPIO_OSPEEDR_OSPEED9_Msk|GPIO_OSPEEDR_OSPEED10_Msk|GPIO_OSPEEDR_OSPEED11_Msk; //high

    GPIOA->AFR[1] &= ~(GPIO_AFRH_AFSEL8|GPIO_AFRH_AFSEL9|GPIO_AFRH_AFSEL10|GPIO_AFRH_AFSEL11);
    GPIOA->AFR[1] |= (GPIO_AFRH_AFSEL8_0|GPIO_AFRH_AFSEL9_0|GPIO_AFRH_AFSEL10_0|GPIO_AFRH_AFSEL11_0); //tim1

    // ------------ configure PWM ------------ 
    TIM1->CR1 |= TIM_CR1_ARPE; // Auto Reload Preload
    TIM1->EGR |= TIM_EGR_UG; // Initial everything

    // ccr config
    TIM1->CCMR1 |= TIM_CCMR1_OC1M_2|TIM_CCMR1_OC1M_1|TIM_CCMR1_OC2M_2|TIM_CCMR1_OC2M_1; // pwm mode 1
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE|TIM_CCMR1_OC2PE; // preload enable
    TIM1->CCMR2 |= TIM_CCMR2_OC3M_2|TIM_CCMR2_OC3M_1|TIM_CCMR2_OC4M_2|TIM_CCMR2_OC4M_1; // pwm mode 1
    TIM1->CCMR2 |= TIM_CCMR2_OC3PE|TIM_CCMR2_OC4PE; // preload enable
    
    // enable
    TIM1->CCER |= TIM_CCER_CC1E|TIM_CCER_CC2E|TIM_CCER_CC3E|TIM_CCER_CC4E;
    TIM1->BDTR |= TIM_BDTR_MOE;
    TIM1->CR1 |= TIM_CR1_CEN; 

    TIM1->ARR = ARR_value;
}

uint16_t _calculate_percent_of_ARR(uint8_t percent){
    uint16_t ARR_value = TIM1->ARR;
    uint16_t percent_of_ARR = ((uint32_t)ARR_value * percent) / 100;
    
    return percent_of_ARR;
}

// set the duty cycle of PWM (% of time on)
void set_pwm_ch1(uint8_t duty_cycle){

    uint16_t value = _calculate_percent_of_ARR(duty_cycle);
    TIM1->CCR1 = value;
}

// set the duty cycle of PWM (% of time on)
void set_pwm_ch2(uint8_t duty_cycle){

    uint16_t value = _calculate_percent_of_ARR(duty_cycle);
    TIM1->CCR2 = value;
}

// set the duty cycle of PWM (% of time on)
void set_pwm_ch3(uint8_t duty_cycle){

    uint16_t value = _calculate_percent_of_ARR(duty_cycle);
    TIM1->CCR3 = value;
}

// set the duty cycle of PWM (% of time on)
void set_pwm_ch4(uint8_t duty_cycle){

    uint16_t value = _calculate_percent_of_ARR(duty_cycle);
    TIM1->CCR4 = value;
}