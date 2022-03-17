#include "adc.h"
#include "lcd.h"
#include "joystick.h"
#include "debug.h"
#include "main.h"

#include <stdio.h>

// in milisecond
const uint32_t joystickMaxDelay = HAL_MAX_DELAY;

// TODO: configure 
const uint32_t x_mid = 2048;
const uint32_t y_mid = 2048;
const uint32_t xy_min = 0;
const uint32_t xy_max = 4096;


//JOYSTICK SHOULD BE USED TO MODULATE THE FREQUENCY 
void readJoystick() {
    while (1) {
        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawy = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawx = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_Stop(&hadc1);

        // normalise the values
        int8_t y = normaliseJoystick(rawy, xy_min, y_mid, xy_max);
        int8_t x = normaliseJoystick(rawx, xy_min, x_mid, xy_max);

        // joystick values are absolute (no need to depend on prev values)
        __atomic_store_n(&pitch,y,__ATOMIC_RELAXED);
        __atomic_store_n(&modulation,x,__ATOMIC_RELAXED);

        // Toggle MCU LED
        HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);

        vTaskDelay( pdMS_TO_TICKS(100) );
    }
}

void init_joystick() {
    if (xTaskCreate(readJoystick, "Read Joystick", 128, NULL, 2, NULL) != pdPASS) {
        DEBUG_PRINT("ERROR");
        print(xPortGetFreeHeapSize());
    }
}

// [-128 - 127]
int8_t normaliseJoystick(uint32_t value, uint32_t min, uint32_t mid, uint32_t max) {
    if (value < mid) {
        value = (value - mid)/mid * 128;
        return (int8_t) value;
    } else {
        value = (value - mid)/(max-mid) * 127;
        return (int8_t) value;
    }
}