# Drone Flight Controller
Making a custom flight controller for a 5-inch drone

### Materials:
    MCU: STM32F411CEU6
    IMU: BMI270
    Receiver: RP1 2.4GHz ELRS
    ESC: SpeedyBee BLS 60A

### To-Do:
1. DSHOT drivers
    - Data packing func
    - Delivering data to DMA func
2. UART drivers
    - CRSF decoder
3. PID

### Things I Learned:
1. DShot
    - Combining DMA and PWM wasnt something I knew was possible