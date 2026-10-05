#ifndef RC_receiver_h
    #define RC_receiver_h
    #include <stdint.h>
    #include "CRSF.h"

    // every control goes from -100 to 100
    typedef struct RC_controls{
        int8_t roll;
        int8_t pitch;
        int8_t throttle;
        int8_t yaw;
        int8_t left_bumper;
        int8_t left_switch;
        int8_t right_switch;
        int8_t right_bumper;
        int8_t left_trigger;
        int8_t right_trigger;
        //int8_t RESERVED[6];
    }RC_controls;

    void RC_receiver_init(uint64_t baudrate, uint64_t SystemClock);
    
    data_validity Receive_RC_controls(RC_controls* data);
    void print_RC_controls(RC_controls* data);
#endif