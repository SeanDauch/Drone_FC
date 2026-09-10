#ifndef pwm_h
    #define pwm_h
    #include <stdint.h>

    void pwm_init(uint16_t ARR_value);
    void set_pwm_ch1(uint8_t duty_cycle);
    void set_pwm_ch2(uint8_t duty_cycle);
    void set_pwm_ch3(uint8_t duty_cycle);
    void set_pwm_ch4(uint8_t duty_cycle);
#endif