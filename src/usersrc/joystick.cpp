#include "adc.h"
#include "lcd.h"
#include "joystick.h"
#include "debug.h"
#include "main.h"
#include "delay.h"

#include <stdio.h>

// in milisecond
// const uint32_t joystickMaxDelay = HAL_MAX_DELAY;

// TODO: configure 
// const int32_t y_mid = 2007;
// const int32_t x_mid = 1790;
// const int32_t xy_min = 0;
// const int32_t xy_max = 4030;


void readJoystick(void* params) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        uint32_t v1, v2;
     
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

extern "C" void init_joystick() {
    DEBUG_PRINT("Initializing Joystick Read");
    HAL_ADC_Start_DMA(&hadc1, &joystick.pitch, 2);
}

// [-128 - 127]
// int8_t normaliseJoystick(uint32_t value, uint32_t min, uint32_t mid, uint32_t max) {
//     int32_t rtn = (int32_t)value;
//     // if (value < mid) {
//     //     // rtn = ((value - mid) * 128) / mid;
//     //     rtn = 1;
//     //     // return (int8_t) value;
//     // } else {
//     //     // rtn =((value - mid) * 127) /(max-mid);
//     //     rtn = 0;
//     //     // return (int8_t) value;
//     // }
//     return rtn ? (int8_t)0 : (int8_t)1;
// }