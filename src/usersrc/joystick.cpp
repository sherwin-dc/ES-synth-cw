#include "adc.h"
#include "lcd.h"
#include "joystick.h"
#include "debug.h"
#include "main.h"
#include "delay.h"

#include <stdio.h>

// in milisecond
const uint32_t joystickMaxDelay = HAL_MAX_DELAY;

// TODO: configure 
const int32_t y_mid = 2007;
const int32_t x_mid = 1790;
const int32_t xy_min = 0;
const int32_t xy_max = 4030;

uint32_t ADC[2];

//JOYSTICK SHOULD BE USED TO MODULATE THE FREQUENCY 
void readJoystick(void* params) {
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        // DEBUG_PRINT("1");
        // delay_microseconds(500);

        // HAL_ADC_Start(&hadc1);
        // HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        // uint32_t rawx = HAL_ADC_GetValue(&hadc1);

        // delay_microseconds(5);
        // for(int i=0; i<100; i++) {
        //     rawx++;
        //     rawx--;
        // };
        // DEBUG_PRINT("2");

        // HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        // uint32_t rawy = HAL_ADC_GetValue(&hadc1);
        

        // HAL_ADC_Stop(&hadc1);
        uint32_t v1, v2;
        v1 = __atomic_load_n(&ADC[0], __ATOMIC_RELAXED);
        v2 = __atomic_load_n(&ADC[1], __ATOMIC_RELAXED);
        // print(v1);
        // print(v2);
        // delay_microseconds(500);
        // DEBUG_PRINT("delay");


        // normalise the values
        // int8_t y = normaliseJoystick(rawy, xy_min, y_mid, xy_max);
        // int8_t x = normaliseJoystick(rawx, xy_min, x_mid, xy_max);

        // // joystick values are absolute (no need to depend on prev values)
        // __atomic_store_n(&pitch,y,__ATOMIC_RELAXED);
        // __atomic_store_n(&modulation,x,__ATOMIC_RELAXED);

        // Toggle MCU LED
        // HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);
        // DEBUG_PRINT("3");

        // vTaskDelay( pdMS_TO_TICKS(100) );
        vTaskDelayUntil( &xLastWakeTime, xFrequency );
    }
}

extern "C" void init_joystick() {
    DEBUG_PRINT("Initializing Joystick Read");
    HAL_ADC_Start_DMA(&hadc1, ADC, 2);


    // if (xTaskCreate(readJoystick, "Read Joystick", 256, NULL, 2, NULL) != pdPASS) {
    //     DEBUG_PRINT("ERROR");
    //     print(xPortGetFreeHeapSize());
    // }
}

// [-128 - 127]
int8_t normaliseJoystick(uint32_t value, uint32_t min, uint32_t mid, uint32_t max) {
    int32_t rtn = (int32_t)value;
    // if (value < mid) {
    //     // rtn = ((value - mid) * 128) / mid;
    //     rtn = 1;
    //     // return (int8_t) value;
    // } else {
    //     // rtn =((value - mid) * 127) /(max-mid);
    //     rtn = 0;
    //     // return (int8_t) value;
    // }
    return rtn ? (int8_t)0 : (int8_t)1;
}