#include "adc.h"
#include "lcd.h"
#include "joystick.h"
#include "debug.h"

#include <stdio.h>

// in milisecond
const uint32_t joystickMaxDelay = HAL_MAX_DELAY;


//JOYSTICK SHOULD BE USED TO MODULATE THE FREQUENCY 
uint8_t readJoystick() {

    while (1) {

        // Read key presses
        lcd_t lcd;

        HAL_ADC_Start(&hadc1);
        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawx = HAL_ADC_GetValue(&hadc1);

        HAL_ADC_PollForConversion(&hadc1, joystickMaxDelay);
        uint32_t rawy = HAL_ADC_GetValue(&hadc1);
        // uint32_t rawy = rawx;

        HAL_ADC_Stop(&hadc1);

        // rawx += (rawy << 16);
        sprintf(lcd,"%lu | %lu",rawx, rawy);

        DEBUG_PRINT("Read Joy Stick");

        u8g2_ClearBuffer(&u8g2);
        u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
        u8g2_DrawStr(&u8g2, 2, 10, lcd);  // write something to the internal memory
        u8g2_SendBuffer(&u8g2);

        // Toggle MCU LED
        HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);

        vTaskDelay( pdMS_TO_TICKS(100) );
    }

    return (uint8_t)0;
}