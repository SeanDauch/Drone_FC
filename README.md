# Drone Flight Controller
Making a custom flight controller for a 5-inch drone

### Materials:
    MCU: STM32F411CEU6
    IMU: BMI270
    Receiver: RP1 2.4GHz ELRS
    ESC: SpeedyBee BLS 60A

### Helpful Resources:
1. https://betaflight.com/docs/development/API/Dshot
2. https://github.com/tbs-fpv/tbs-crsf-spec/blob/main/crsf.md

### To-Do:
1. ~~DSHOT drivers~~
    - ~~Data packing func~~
    - ~~Delivering data to DMA func~~
2. UART drivers
    - ~~Add DMA to UART~~
    - ~~CRSF decoder~~
    - Implement double buffer mode for reciever
    - add CRC
3. PID

### Things I Learned:
1. DShot
    - Combining DMA and PWM wasnt something I knew was possible
    - I learned a lot more about how different DMA options are used
2. UART
    - Ive never used uart before so I learned how to set up drivers
    - I used the uart drivers to send data to my pc using the serial monitor
3. SWO
    - I learn about Serial wire output and made my own SWO drivers for debugging
    - I learned more about VSCode configs and how OpenOCD is structed