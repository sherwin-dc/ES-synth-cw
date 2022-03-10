#include "gpio.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keymat.h"

QueueHandle_t boardkeys;
QueueHandle_t boardknobs; 

void setOutMuxBit(const uint8_t bitIdx, const int value) {
      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_0_GPIO_Port, Row_Sel_0_Pin, (bitIdx & 0x01) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_1_GPIO_Port, Row_Sel_1_Pin, (bitIdx & 0x02) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_2_GPIO_Port, Row_Sel_2_Pin, (bitIdx & 0x04) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(GPIOB, GPIO_PIN_5, value ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_SET);
      delay_microseconds(2);
      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_RESET);
}

// read inputs from C0-C3 with four bits concatenated at the LSBs
// return value [0,F]
uint8_t readCols() {
      uint8_t rtn = 0x00;

      int state = HAL_GPIO_ReadPin(C0_GPIO_Port, C0_Pin);
      if (state == 1) {
            rtn |= (uint8_t)0x01;
      }
      state = HAL_GPIO_ReadPin(C1_GPIO_Port, C1_Pin);
      if (state == 1) {
            rtn |= (uint8_t)0x02;
      }
      state = HAL_GPIO_ReadPin(C2_GPIO_Port, C2_Pin);
      if (state == 1) {
            rtn |= (uint8_t)0x04;
      }
      state = HAL_GPIO_ReadPin(C3_GPIO_Port, C3_Pin);
      if (state == 1) {
            rtn |= (uint8_t)0x08;
      }

      return rtn;
}

// set the switch matrix values
// rodIdx range: [0,7]
void setRow(uint8_t rowIdx) {
      if (rowIdx > 7) {
            return; // some kind of error handling?
      }

      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_0_GPIO_Port, Row_Sel_0_Pin, (rowIdx & 0x01) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_1_GPIO_Port, Row_Sel_1_Pin, (rowIdx & 0x02) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_2_GPIO_Port, Row_Sel_2_Pin, (rowIdx & 0x04) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_SET);
}

// increment boardknobs according to rotation
// no overflow implemented
// shall this be incrementing or boolean?
// just a function or a class?
void knobDecode(uint8_t* newKeys) {
      boardkeys_t keys;
      xQueuePeek(boardkeys, &keys, 0);

      knob_t knobsRotation;
      xQueuePeek(boardknobs, &knobsRotation, 0);

      // somehow the knobs read backward
      for (int i=0; i<4; i++){
            // {0,0,prevB,prevA, 0,0,currB,currA}
            // assert all values are 0 or 1?
            // super hacky as well
            uint8_t state = (keys[12+i*2]<<4) + (keys[13+i*2]<<5);
            state += (newKeys[i*2]) + (newKeys[i*2+1]<<1);

            switch (state){
                  case 0x01:
                        knobsRotation[3-i] += 1;
                        break;
                  case 0x02:
                        knobsRotation[3-i] -=1;
                        break;
                  case 0x10:
                        knobsRotation[3-i] -= 1;
                        break;
                  case 0x13:
                        knobsRotation[3-i] += 1;
                        break;
                  case 0x20:
                        knobsRotation[3-i] += 1;
                        break;
                  case 0x23:
                        knobsRotation[3-i] -= 1;
                        break;
                  case 0x31:
                        knobsRotation[3-i] -= 1;
                        break;
                  case 0x32:
                        knobsRotation[3-i] += 1;
                        break;
                  default:
                        break;
            }
      }

      xQueueOverwrite(boardknobs, &knobsRotation);
}

// main function that scan keys
void scanKeysTask(void * params) {
      while (1) {
            //Testing keypresses
            boardkeys_t keyPressed;

            // flip bits for keyboard keys (active LOW)
            int i=0;
            for(;i <= 2; i++){
                  setRow(i);
                  delay_microseconds(3);
                  uint8_t keys = readCols();
                  keyPressed[i*4]   = !(keys & (uint8_t)0x01);
                  keyPressed[i*4+1] = !(keys & (uint8_t)0x02);
                  keyPressed[i*4+2] = !(keys & (uint8_t)0x04);
                  keyPressed[i*4+3] = !(keys & (uint8_t)0x08);
            }
            
            for (;i < 7; i++){
                  setRow(i);
                  delay_microseconds(3);
                  uint8_t keys = readCols();
                  keyPressed[i*4]   = keys & (uint8_t)0x01;
                  keyPressed[i*4+1] = keys & (uint8_t)0x02 >> 1;
                  keyPressed[i*4+2] = keys & (uint8_t)0x04 >> 2;
                  keyPressed[i*4+3] = keys & (uint8_t)0x08 >> 3;
            }

            // update knobs
            knobDecode(keyPressed + 12); // this is a bit hacky, need testing

            xQueueOverwrite(boardkeys, &keyPressed);

            vTaskDelay( pdMS_TO_TICKS(100) );
      }
}

void init_keydetect() {
      boardkeys = xQueueCreate(1, sizeof(boardkeys_t));
      DEBUG_PRINT("Initialising Detect Keys");
      if (xTaskCreate(scanKeysTask, "Detect keys", 512, NULL, 5, NULL) != pdPASS) {
      DEBUG_PRINT("ERROR")
      }
}