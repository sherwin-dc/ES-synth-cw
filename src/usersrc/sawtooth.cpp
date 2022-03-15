
#include "dac.h"
#include "debug.h"
#include "tim.h"
#include "dma.h"
#include "main.h"

#include <math.h> // sin()
#include <algorithm> // std::copy
#include <cstring> // Contains the memcpy function


//const float frequencies [12] = {261.63,277.18,293.66,311.13,329.63,349.23,370.00,392.00,415.30,440.00,466.16,493.88}
//const int32_t stepSizes[12] = {51076922, 54112683, 57330004, 60740598, 64352275, 68178701, 72233540, 76528508, 81077269, 85899345, 91006452, 96418111};


// Number of 22 kHz clocks cycles in one period of a given note (Calculate index by 12*octave + index of note)
const float periods[9*12] = {
  1223.1,1154.5,1089.7,1028.5,970.79,916.3,864.86,816.33,770.53,727.27,686.46,647.93,
  611.55,577.24,544.85,514.25,485.39,458.15,432.43,408.16,385.26,363.64,343.23,323.97,
  305.78,288.62,272.42,257.13,242.7,229.08,216.22,204.08,192.63,181.82,171.61,161.98,
  152.89,144.31,136.21,128.56,121.35,114.54,108.11,102.04,96.316,90.909,85.807,80.991,
  76.444,72.155,68.106,64.282,60.674,57.269,54.054,51.02,48.158,45.455,42.904,40.496,
  38.222,36.078,34.053,32.141,30.337,28.634,27.027,25.51,24.079,22.727,21.452,20.248,
  19.111,18.039,17.026,16.07,15.169,14.317,13.514,12.755,12.039,11.364,10.726,10.124,
  9.5555,9.0194,8.5132,8.0352,7.5843,7.1586,6.7568,6.3776,6.0197,5.6818,5.363,5.062,
  4.7777,4.5097,4.2566,4.0176,3.7921,3.5793,3.3784,3.1888,3.0099,2.8409,2.6815,2.531
};

uint32_t steps [2200] = {0}; // Array which holds the data which the DMA gives the DAC

// Overwrite section of array read by DMA, region determines which section (either 0 or 1) 
extern "C" void sampleSound(uint8_t region){
  uint32_t tmpSteps [1100] = {0}; // Array to temporarily hold the values which we write to steps
  
  // Create a copy of the playedNotes array
  uint8_t tmpNotes [9*12];
  // NEED TO FIND SOMEHOW TO PROTECT MEMORY
  //xSemaphoreTake(playedNotesMutex, portMAX_DELAY);
  memcpy(tmpNotes,(void*)playedNotes,sizeof(tmpNotes));
  //xSemaphoreGive(playedNotesMutex);

  // Read in the volume
  uint8_t tmpVolume = __atomic_load_n(&volume,__ATOMIC_RELAXED);

  // Calculate what the array should be for the next iteration
  switch(__atomic_load_n(&sound,__ATOMIC_RELAXED)) {
    case 1: // Sine wave
      for(int i=0; i<9*12; i++){
        if(tmpNotes[i]){
          for(int j=0; j<1100; j++){
            //tmpSteps[j] += (float(tmpVolume)/9)*4096*(fmod(j,periods[i])/periods[i]);
            //tmpSteps[j] = 2048*(float(tmpVolume)/9)*sin(periods[i]*j*2*3.14159) + 2048;
          }
          break;
        }
      }
      break;
    case 2:
      // code block
      break;
    default: // Sawtooth
      for(int i=0; i<9*12; i++){
        if(tmpNotes[i]){
          for(int j=0; j<1100; j++){
            tmpSteps[j] = (float(tmpVolume)/9)*4096*(fmod(j,periods[i])/periods[i]);
          }
          break;
        }
      }
  }

  // Copy array to memory used by DMA 
  std::copy(tmpSteps, tmpSteps + 1100, steps + region*1100);


  DEBUG_PRINT("Writing new array");

    //HAL_DAC_SetValue(GPIOA,DAC_CHANNEL_1,GPIO_PIN_4,);
    //HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_8B_R,Vout + 128);
    //HAL_DAC_SetValue(&hdac1,DAC_CHANNEL_1,DAC_ALIGN_8B_R,128);

    //uint8_t var = 1.2*(4096)/3.3;
    //HAL_DAC_SetValue(&hdac1, DAC_CHANNEL_1, DAC_ALIGN_12B_R, var);

}

extern "C" void init_sound() {
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) steps, 2200, DAC_ALIGN_12B_R);
  HAL_TIM_Base_Start(&htim2);

  HAL_Delay(500);
  __HAL_TIM_SET_AUTORELOAD(&htim2, 3273); // Set DMA to work at 22 kHz

  //HAL_Delay(500);
  //HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

}

void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  sampleSound(0); // Overwrite first half of steps array
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  sampleSound(1); // Overwrite second half of steps array
}
