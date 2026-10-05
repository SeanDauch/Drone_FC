#include "stm32f4xx.h"
#include "CRSF.h"
#include "RC_receiver.h"
#include "debugging.h"
#include <stdio.h>

typedef enum{
    waiting_for_start,
    init_complete
}init_status;

void RC_receiver_init(uint64_t baudrate, uint64_t SystemClock){

    CRSF_init(baudrate, SystemClock);

    // loop until clear data is found
    init_status current_status = waiting_for_start;
    RC_controls temp_RC_data = {0};

    while(current_status == waiting_for_start){
       if(Receive_RC_controls(&temp_RC_data) == good_data){
        current_status = init_complete;
       }
    }
}

#define RC_stick_min 174
#define RC_stick_mid 992
#define RC_stick_max 1811
#define RC_stick_formater(data) (data - RC_stick_mid)/8.18f

#define RC_button_min 191
#define RC_button_mid 997
#define RC_button_max 1792
#define RC_button_formater(data) (data/8) - 123.875f // linear regression in desmos



RC_controls _format_RC_channels(RC_Data* unformated_ch){
    RC_controls new_controls = {
        RC_stick_formater(unformated_ch->channel_01),
        RC_stick_formater(unformated_ch->channel_02),
        RC_stick_formater(unformated_ch->channel_03),
        RC_stick_formater(unformated_ch->channel_04),
        RC_button_formater(unformated_ch->channel_05),
        RC_button_formater(unformated_ch->channel_06),
        RC_button_formater(unformated_ch->channel_07),
        RC_button_formater(unformated_ch->channel_08),
        RC_button_formater(unformated_ch->channel_09),
        RC_button_formater(unformated_ch->channel_10),
    };

    return new_controls;
}

// RC_data will be unchanged in case of communication error
data_validity Receive_RC_controls(RC_controls* previous_controller_data){

    uint8_t temp_data_buffer[rc_frame_len] = {0};

    if(receive_RC_CRSF_data(temp_data_buffer, rc_frame_len) == bad_data){
        return bad_data;
    }

    // --------------- validate CRC --------------------
    uint8_t expected_CRC =  generate_crc(&temp_data_buffer[type_byte_pos], rc_frame_len-type_byte_pos-1);
    if(temp_data_buffer[CRC_byte_pos] != expected_CRC){
        return bad_data;
    }

    // -------------- unpack and format data -----------------
    RC_Data unformated_data = unpack_rc_CRSF_data(&temp_data_buffer[data_start_pos], rc_data_len);
    *previous_controller_data = _format_RC_channels(&unformated_data);

    return good_data;
}

void print_RC_controls(RC_controls* data){
    printf("roll: %d\n", data->roll);
    printf("pitch: %d\n", data->pitch);
    printf("throttle: %d\n", data->throttle);
    printf("yaw: %d\n", data->yaw);
    printf("left_bumper: %d\n", data->left_bumper);
    printf("left_switch: %d\n", data->left_switch);
    printf("right_switch: %d\n", data->right_switch);
    printf("right_bumper: %d\n", data->right_bumper);
    printf("left_trigger: %d\n", data->left_trigger);
    printf("right_trigger: %d\n", data->right_trigger);
    printf("---------------------------\n");
}