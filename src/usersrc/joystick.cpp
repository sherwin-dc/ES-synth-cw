
#include "joystick.h"
#include "main.h"
#include "adc.h"

extern "C" void init_joystick() {
    DEBUG_PRINT("Initializing Joystick Read");
    HAL_ADC_Start_DMA(&hadc1,(uint32_t *) &joystick.pitch, 2);
}

