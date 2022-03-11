#include "adc.h"
#include "lcd.h"
#include "joystick.h"

#include <stdio.h>

// in milisecond
const uint32_t joystickMaxDelay = HAL_MAX_DELAY;


//JOYSTICK SHOULD BE USED TO MODULATE THE FREQUENCY 
uint8_t readJoystick() {
    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 1);
    uint32_t raw = HAL_ADC_GetValue(&hadc1);

    // convert to char[]
    lcd_t lcd;
    sprintf(lcd,"%lu",raw);
    xQueueOverwrite(lcdQueue, &lcd);

    return (uint8_t)0;
}