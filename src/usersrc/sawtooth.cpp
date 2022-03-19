
#include "dac.h"
#include "debug.h"
#include "tim.h"
#include "dma.h"
#include "main.h"

#include <algorithm> // std::copy


// Step sizes needed to overflow int32_t at correct frequency if accumulated at 22 kHz
//(Calculate index by 12*octave + index of note)
const int32_t stepSizes [9*12] = { 
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
const int32_t sine [256] = {
  0,52701876,105372008,157978672,210490160,262874864,315101248,367137792,
418953184,470516224,521795872,572761152,623381504,673626240,723465344,772868544,
821806272,870248896,918167424,965532800,1012316608,1058490624,1104027008,1148898432,
1193077760,1236538496,1279254272,1321199488,1362348928,1402677760,1442161536,1480776832,
1518499968,1555308544,1591180160,1626093312,1660027008,1692960896,1724874752,1755749760,
1785567104,1814308992,1841958016,1868497408,1893911168,1918184320,1941302016,1963250304,
1984016000,2003586560,2021950336,2039096064,2055013504,2069693184,2083126144,2095304320,
2106220288,2115867520,2124240256,2131333504,2137142912,2141664896,2144896896,2146836864,
2147483647,2146836864,2144896896,2141665024,2137143040,2131333632,2124240512,2115867776,
2106220544,2095304576,2083126400,2069693568,2055014016,2039096448,2021950720,2003587200,
1984016512,1963250944,1941302656,1918184960,1893911808,1868498176,1841958656,1814309632,
1785568000,1755750656,1724875520,1692961792,1660028032,1626094208,1591180928,1555309568,
1518500992,1480777728,1442162816,1402678784,1362349952,1321200768,1279255424,1236539520,
1193079168,1148899712,1104028160,1058492032,1012317888,965533952,918168448,870250304,
821807488,772869696,723466752,673627584,623382656,572762688,521797216,470517472,
418954752,367139200,315102496,262876496,210491632,157979984,105373160,52703384,
1348,-52700688,-105370472,-157977296,-210488944,-262873312,-315099840,-367136544,
-418951616,-470514816,-521794624,-572759616,-623380096,-673625024,-723464192,-772867136,
-821804992,-870247872,-918166016,-965531584,-1012315520,-1058489216,-1104025856,-1148897408,
-1193076480,-1236537216,-1279253248,-1321198208,-1362347776,-1402676736,-1442160384,-1480775680,
-1518499072,-1555307648,-1591179136,-1626092160,-1660025984,-1692959872,-1724873984,-1755749120,
-1785566592,-1814308480,-1841956992,-1868496512,-1893910528,-1918183808,-1941301504,-1963249920,
-1984015232,-2003585920,-2021949824,-2039095680,-2055013248,-2069692928,-2083125632,-2095303808,
-2106219904,-2115867264,-2124240128,-2131333376,-2137142784,-2141664768,-2144896768,-2146836864,
-2147483648,-2146836864,-2144897024,-2141665152,-2137143168,-2131333888,-2124240640,-2115867904,
-2106220672,-2095304704,-2083126912,-2069693952,-2055014400,-2039096960,-2021951104,-2003587456,
-1984017280,-1963251584,-1941303296,-1918185600,-1893912448,-1868498560,-1841959680,-1814310656,
-1785568768,-1755751424,-1724876416,-1692962304,-1660028544,-1626095488,-1591182208,-1555310464,
-1518501888,-1480778624,-1442163456,-1402680192,-1362351360,-1321201920,-1279256576,-1236540544,
-1193079808,-1148901248,-1104029696,-1058493184,-1012319104,-965535168,-918169664,-870251072,
-821809216,-772871424,-723468032,-673628864,-623383936,-572763520,-521799040,-470519264,
-418956096,-367140544,-315103840,-262877328,-210492464,-157981840,-105375024,-52704732
};

int32_t accumulators [9*12] = {0}; // Array which holds an accumulator for all notes
int32_t chorusAccumulators [9*12*4] = {0}; // Array which holds extra accumulators used in chorus mode

uint32_t steps [1100] = {0}; // Array which holds the data which the DMA gives the DAC
int32_t stepsNOFF [1100] = {0}; // Array with same data as steps but centred around 0

int32_t reverbArray [3300] = {0}; // Array which holds the data which the reverb function uses
uint8_t reverbIndex = 0; // Keeps track of where in the array we should write data next



// Overwrite section of array read by DMA, region determines which section (either 0 or 1) 
extern "C" void sampleSound(uint8_t region){

  uint32_t tmpSteps [550] = {0}; // Array to temporarily hold the values which we write to steps
  int32_t tmpStepsNOFF [550] = {0}; // Array to temporarily hold the same data as steps but centred around 0

  // Read in the volume
  uint8_t tmpVolume = __atomic_load_n(&volume,__ATOMIC_RELAXED);


  // Calculate what the array should be for the next iteration
  switch(__atomic_load_n(&sound,__ATOMIC_RELAXED)) {
    case 0: // Sawtooth
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpStepsNOFF[j] = accumulators[i] >> (27-tmpVolume);
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
            tmpStepsNOFF[j] += accumulators[i] >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
          }
        }
      }
      break;
    case 2: // Chorus
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpStepsNOFF[j] += int32_t(0.70F*accumulators[i]) >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
            tmpStepsNOFF[j] += int32_t(0.5F*chorusAccumulators[i]) >> (27-tmpVolume);
            chorusAccumulators[i] += int32_t(0.98810F*stepSizes[i]);
            tmpStepsNOFF[j] += int32_t(0.5F*chorusAccumulators[i+108]) >> (27-tmpVolume);
            chorusAccumulators[i+108] += int32_t(1.01189F*stepSizes[i]);
            
          }
        }
      }
      break;
    case 3: // LASER
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpStepsNOFF[j] += int32_t(0.63F*accumulators[i]) >> (27-tmpVolume);
            accumulators[i] += stepSizes[i];
            tmpStepsNOFF[j] += int32_t(0.316F*chorusAccumulators[i]) >> (27-tmpVolume);
            chorusAccumulators[i] += int32_t(0.98810F*stepSizes[i]);
            tmpStepsNOFF[j] += int32_t(0.447F*chorusAccumulators[i+108]) >> (27-tmpVolume);
            chorusAccumulators[i+108] += int32_t(0.99405F*stepSizes[i]);
            tmpStepsNOFF[j] += int32_t(0.447F*chorusAccumulators[i+2*108]) >> (27-tmpVolume);
            chorusAccumulators[i+2*108] += int32_t(1.00594F*stepSizes[i]);
            tmpStepsNOFF[j] += int32_t(0.316F*chorusAccumulators[i+3*108]) >> (27-tmpVolume);
            chorusAccumulators[i+3*108] += int32_t(1.01189F*stepSizes[i]);
          }
        }
      }
      break;
    case 4: // Sinewave
      for(int i=0; i<9*12; i++){
        if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
          for(int j=0; j<550; j++){
            tmpStepsNOFF[j] += sine[uint32_t(accumulators[i]+4294967295/2)>>25] >> (27-tmpVolume);
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
        tmpStepsNOFF[i] *= 1.0F-float(tmpReverb)/10;
        tmpStepsNOFF[i] += (float(tmpReverb)/10)*reverbArray[i+reverbIndex*550];
      }

    // Copy tmpStepsNOFF to queue used by reverb
    std::copy(tmpStepsNOFF, tmpStepsNOFF + 550, reverbArray + reverbIndex*550);

    if(reverbIndex >= 5){
      reverbIndex = 0;
    }else{
      reverbIndex++;
    }
  }

  // Offset tmpStepsNOFF to create tmpSteps
  for(int i = 0; i <550; i++){
    tmpSteps[i] = uint32_t(tmpStepsNOFF[i] + 2048);
  }

  // Copy array to memory used by DMA 
  std::copy(tmpSteps, tmpSteps + 550, steps + region*550);

}



extern "C" void init_sound() {
  HAL_DAC_Start_DMA(&hdac1, DAC_CHANNEL_1, (uint32_t *) steps, 1100, DAC_ALIGN_12B_R);
  HAL_TIM_Base_Start(&htim7);

  HAL_Delay(500);
  __HAL_TIM_SET_AUTORELOAD(&htim7, 3273); // Set DMA to work at 22 kHz

  //HAL_Delay(500);
  //HAL_DAC_Stop_DMA(&hdac1, DAC_CHANNEL_1);

}



void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  sampleSound(0); // Overwrite first half of steps array
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
  sampleSound(1); // Overwrite second half of steps array
}
