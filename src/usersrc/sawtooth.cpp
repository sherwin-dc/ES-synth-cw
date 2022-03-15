
#include "dac.h"
#include "debug.h"
#include "tim.h"
#include "dma.h"
#include "main.h"

#include <math.h>
#include <algorithm> // std::copy
#include <cstring> // Contains the memcpy function


// Step sizes needed to overflow uint32_t at correct frequency if accumulated at 22 kHz
//(Calculate index by 12*octave + index of note)
const uint32_t stepSizes [9*12] = { 
  3511538,3720246,3941437,4175916,4424219,4687285,4966055,5261334,5574062,5905580,6256693,6628745,
  7023076,7440493,7882875,8351832,8848438,9374571,9932111,10522669,11148124,11811160,12513387,13257490,
  14046153,14880987,15765751,16703664,17696876,18749143,19864223,21045339,22296248,23622320,25026774,26514980,
  28092307,29761975,31531502,33407329,35393752,37498286,39728447,42090679,44592496,47244640,50053549,53029961,
  56184615,59523950,63063005,66814659,70787504,74996573,79456894,84181358,89184993,94489280,100107098,106059923,
  112369231,119047900,126126011,133629319,141575009,149993147,158913789,168362717,178369986,188978560,200214196,212119846,
  224738462,238095800,252252022,267258639,283150018,299986295,317827579,336725435,356739973,377957121,400428393,424239693,
  449476925,476191601,504504044,534517278,566300036,599972590,635655159,673450871,713479946,755914243,800856787,848479387,
  898953851,952383202,1009008089,1069034556,1132600072,1199945180,1271310319,1346901743,1426959892,1511828487,1601713575,1696958774
};

// Array holding one period of sinf()
const uint32_t sine [128] = {
  0,2586752,10340736,23243392,41263360,64357504,92470144,125533312,
  163467648,206181632,253572480,305525632,361916544,422608896,487456640,556303488,
  628983680,705322112,785134720,868229376,954405888,1043456640,1135166976,1229316224,
  1325677312,1424018304,1524102144,1625687808,1728530432,1832382464,1936993536,2042111616,
  2147483648,2252855552,2357973760,2462584832,2566436864,2669279488,2770865152,2870948864,
  2969289984,3065651200,3159800320,3251510784,3340561408,3426737920,3509832704,3589645312,
  3665983488,3738663936,3807510528,3872358400,3933050880,3989441536,4041394688,4088785664,
  4131499520,4169434112,4202497024,4230609920,4253703936,4271724032,4284626432,4292380672,
  4294967295,4292380672,4284626688,4271724032,4253704192,4230609920,4202497536,4169434368,
  4131500032,4088786432,4041395456,3989442304,3933051648,3872359168,3807511552,3738664448,
  3665984512,3589646336,3509833728,3426739200,3340562944,3251511808,3159801600,3065652224,
  2969291264,2870950400,2770866176,2669280768,2566438400,2462586112,2357975296,2252856832,
  2147484928,2042113152,1936994688,1832383744,1728531968,1625689088,1524103552,1424019456,
  1325678592,1229317632,1135168128,1043457792,954407168,868230400,785135872,705323264,
  628984576,556304512,487457664,422609664,361917056,305526656,253573120,206182144,
  163468416,125533824,92470400,64358016,41263744,23243520,10340864,2586880
};

uint32_t accumulators [9*12] = {0}; // Array which holds an accumulator for all notes

uint32_t chorusAccumulators [9*12*4] = {0}; // Array which holds extra accumulators used in chorus mode


uint32_t steps [1100] = {0}; // Array which holds the data which the DMA gives the DAC

uint32_t reverbArray [3300] = {0}; // Array which holds the data which the reverb function uses
uint8_t reverbIndex = 0; // Keeps track of where in the array we should write data next

// Overwrite section of array read by DMA, region determines which section (either 0 or 1) 
extern "C" void sampleSound(uint8_t region){
  uint32_t tmpSteps [550] = {0}; // Array to temporarily hold the values which we write to steps

  // Read in the volume
  uint8_t tmpVolume = __atomic_load_n(&volume,__ATOMIC_RELAXED);

  // Calculate what the array should be for the next iteration
  switch(__atomic_load_n(&sound,__ATOMIC_RELAXED)) {
    case 0: // Sawtooth
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpSteps[j] = accumulators[i] >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
          }
          break;
        }
      }
      break;
    case 1: // Polyphony
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpSteps[j] += accumulators[i] >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
          }
        }
      }
      break;
    case 2: // Chorus
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpSteps[j] += uint32_t(0.70*float(accumulators[i])) >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
            tmpSteps[j] += uint32_t(0.5*float(chorusAccumulators[i])) >> (27-tmpVolume);
            chorusAccumulators[i] += uint32_t(0.98810*float(stepSizes[i]));
            tmpSteps[j] += uint32_t(0.5*float(chorusAccumulators[i+108])) >> (27-tmpVolume);
            chorusAccumulators[i+108] += uint32_t(1.01189*float(stepSizes[i]));
            
          }
        }
      }
      break;
    case 3: // LASER
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpSteps[j] += uint32_t(0.63*float(accumulators[i])) >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
            tmpSteps[j] += uint32_t(0.316*float(chorusAccumulators[i])) >> (27-tmpVolume);
            chorusAccumulators[i] += uint32_t(0.98810*float(stepSizes[i]));
            tmpSteps[j] += uint32_t(0.447*float(chorusAccumulators[i+108])) >> (27-tmpVolume);
            chorusAccumulators[i+108] += uint32_t(0.99405*float(stepSizes[i]));
            tmpSteps[j] += uint32_t(0.447*float(chorusAccumulators[i+2*108])) >> (27-tmpVolume);
            chorusAccumulators[i+2*108] += uint32_t(1.00594*float(stepSizes[i]));
            tmpSteps[j] += uint32_t(0.316*float(chorusAccumulators[i+3*108])) >> (27-tmpVolume);
            chorusAccumulators[i+3*108] += uint32_t(1.01189*float(stepSizes[i]));
          }
        }
      }
      break;
    case 4: // Sinewave
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpSteps[j] += sine[accumulators[i]>>26] >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
          }
        }
      }
      break;
    default:
      // Do nothing
      DEBUG_PRINT("CASE DEFAULT")
      break;
  }

  // Add reverb
  uint8_t tmpReverb = __atomic_load_n(&reverb,__ATOMIC_RELAXED); // Read in the reverb
    if(tmpReverb > 0){
      for(int i=0; i<550; i++){
      tmpSteps[i] += (float(tmpReverb)/10)*float(reverbArray[i+reverbIndex*550]);
    }

    // Copy array to queue used by reverb
    std::copy(tmpSteps, tmpSteps + 550, reverbArray + reverbIndex*550);

    if(reverbIndex >= 5){
      reverbIndex = 0;
    }else{
      reverbIndex++;
    }
  }

  // Copy array to memory used by DMA 
  std::copy(tmpSteps, tmpSteps + 550, steps + region*550);

}

extern "C" void init_sound() {
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) steps, 1100, DAC_ALIGN_12B_R);
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
