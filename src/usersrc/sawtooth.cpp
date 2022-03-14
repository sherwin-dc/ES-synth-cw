
#include "dac.h"
#include "debug.h"
#include "tim.h"
#include "dma.h"


//const float frequencies [12] = {261.63,277.18,293.66,311.13,329.63,349.23,370.00,392.00,415.30,440.00,466.16,493.88}
const int32_t stepSizes[12] = {51076922, 54112683, 57330004, 60740598, 64352275, 68178701, 72233540, 76528508, 81077269, 85899345, 91006452, 96418111};
volatile int32_t currentStepSize;

extern "C" void sampleISR(){
    static int32_t phaseAcc = 0;

    phaseAcc += 51076922;
    //phaseAcc+= currentStepSize;
    int32_t Vout = phaseAcc >> 24;


    // DEBUG_PRINT("hello world");

    //HAL_DAC_SetValue(GPIOA,DAC_CHANNEL_1,GPIO_PIN_4,);
    //HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_8B_R,Vout + 128);
    //HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_8B_R,128);

    //uint8_t var = 1.2*(4096)/3.3;
    //HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, var);
}

extern "C" void init_sound() {
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) stepSizes, 12, DAC_ALIGN_12B_R);
  HAL_TIM_Base_Start(&htim2);

  HAL_Delay(500);
  __HAL_TIM_SET_AUTORELOAD(&htim2, 4000);

  HAL_Delay(500);
  __HAL_TIM_SET_AUTORELOAD(&htim2, 2000);

  HAL_Delay(500);
  HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

  
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  // DEBUG_PRINT("FULL")
}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  // DEBUG_PRINT("HALF")
}


/*
//const int32_t frequencies [12] = {261.63,277.18,293.66,311.13,329.63,349.23,370.00,392.00,415.30,440.00,466.16,493.88}
const int32_t stepSizes[12] = {51076922, 54112683, 57330004, 60740598, 64352275, 68178701, 72233540, 76528508, 81077269, 85899345, 91006452, 96418111};

volatile int32_t currentStepSize;
static int32_t phaseAcc= 0;


void sampleISR() {
    phaseAcc+= currentStepSize;

    int32_t Vout = phaseAcc >> 24;


}
*/
/*extern "C" void update_lcd() {
  u8g2_ClearBuffer(&u8g2);
  u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
  u8g2_DrawStr(&u8g2, 2, 10,"Helllo World!");  // write something to the internal memory
  u8g2_SendBuffer(&u8g2);*/
