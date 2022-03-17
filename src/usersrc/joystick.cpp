#include "adc.h"
#include "lcd.h"
#include "joystick.h"
#include "debug.h"

#include <stdio.h>

// in milisecond
const uint32_t joystickMaxDelay = HAL_MAX_DELAY;

// TODO: configure 
const uint32_t x_mid = 2048;
const uint32_t y_mid = 2048;
const uint32_t xy_min = 0;
const uint32_t xy_max = 4096;


//JOYSTICK SHOULD BE USED TO MODULATE THE FREQUENCY 
uint8_t readJoystick() {
    while (1) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawx = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawy = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_Stop(&hadc1);

        // Toggle MCU LED
        HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);

        vTaskDelay( pdMS_TO_TICKS(100) );
    }
    return (uint8_t)0;
}

void init_joystick() {
    
}