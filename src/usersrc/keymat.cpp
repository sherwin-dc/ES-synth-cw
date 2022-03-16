#include "gpio.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keymat.h"

// for debug only
#include "lcd.h"
#include <stdio.h>

#include "main.h"

#include <cstring> // Contains the memcpy function
#include <algorithm> // Contains max and min functions

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


// Check whether knobs are being turned
// ! Also detects whether keys state has been changed
void knobDecode(boardkeys_t newKeys, uint8_t* TX_Message_Ptr) {

      // Static variable holding the keys from the last time the function was called
      static boardkeys_t oldKeys = {0};

      // Array which hold the information whether a knob is turned to the left (-1) or right (1), or not at all (0)
      int8_t knobRotation [4] = {0};

      // Check whether any of the keys have been moved forward or backwards
      // We only care about transitions from states 00 or 11, to states 01 or 10.
      for (int i=0; i<4; i++){
            uint8_t oldState = (oldKeys[12+i*2]<<1) + (oldKeys[13+i*2]); // Calculate previous state
            uint8_t newState = (newKeys[12+i*2]<<1) + (newKeys[13+i*2]); // Calculate current state

            switch (oldState){
                  case 0x00:
                        if(newState == 0x01){
                              knobRotation[i] = -1; // Turning left
                        }else if(newState == 0x02){
                              knobRotation[i] = 1; // Turning right
                        }
                        break;
                  case 0x03:
                        if(newState == 0x01){
                              knobRotation[i] = 1; // Turning right
                        }else if(newState == 0x02){
                              knobRotation[i] = -1; // Turning left
                        }
                  default:
                        break;
            }
      }

      // Check for changes in key presses
      uint8_t anyKeyPressed = false;
      for (int i=0; i<12; i++){
            uint8_t oldKeyState = oldKeys[i];
            uint8_t newKeyState = newKeys[i];

            // button state has changed
            if (oldKeyState ^ newKeyState) {
                  anyKeyPressed = true;
                  // TODO: Add this to a queue so that multiple simultaneous key presses are also reflected
                  // TX_Message_Ptr[1] = i;                  
                  // TX_Message_Ptr[2] = octave;                  
                  
                  TX_Message_Ptr[1] = i + 48;
                  TX_Message_Ptr[2] = octave + 48;

                  // Button has been pressed
                  if (oldKeyState==1)      { TX_Message_Ptr[0] = 'P'; }
                  // Button has been released
                  else if (oldKeyState==0) { TX_Message_Ptr[0] = 'R'; }

                  DEBUG_PRINT("TX CAN MSG");
                  CAN_TX(0x123, TX_Message_Ptr);      // Transmit message over CAN
            } 
      }
      
      // Clear tx buffer
      if (!anyKeyPressed) {
            TX_Message_Ptr[0] = ' ';
            TX_Message_Ptr[1] = ' ';
            TX_Message_Ptr[2] = ' ';
      }

      // Update related global variables
      if(knobRotation[3] != 0){ // Update volume
            int tmpVolume= int(__atomic_load_n(&volume,__ATOMIC_RELAXED)) + knobRotation[3];
            tmpVolume = std::min(std::max(int(tmpVolume),0),7);
            __atomic_store_n(&volume,tmpVolume,__ATOMIC_RELAXED);
      }

      if(knobRotation[2] != 0){ // Update octave and reset notes played
            int tmpOctave= int(__atomic_load_n(&octave,__ATOMIC_RELAXED)) + knobRotation[2];
            tmpOctave = std::min(std::max(int(tmpOctave),0),8);
            __atomic_store_n(&octave,tmpOctave,__ATOMIC_RELAXED);

            //Reset notes played (This is to make sure that no note in the notesPlayed array will never
            // turn off if we change the octave while a note is being played)
            for(int i=0; i<9*12; i++){
                  __atomic_store_n(&playedNotes[i],0,__ATOMIC_RELAXED);
            }

      }

      if(knobRotation[1] != 0){ // Update sound
            int tmpSound= int(__atomic_load_n(&sound,__ATOMIC_RELAXED)) + knobRotation[1];
            tmpSound = std::min(std::max(int(tmpSound),0),9);
            __atomic_store_n(&sound,tmpSound,__ATOMIC_RELAXED);
      }

      if(knobRotation[0] != 0){ // Update reverb
            int tmpReverb = int(__atomic_load_n(&reverb,__ATOMIC_RELAXED)) + knobRotation[0];
            tmpReverb = std::min(std::max(int(tmpReverb),0),9);
            __atomic_store_n(&reverb,tmpReverb,__ATOMIC_RELAXED);
      }

      // Copy newKeys into oldKeys
      memcpy(oldKeys,newKeys,sizeof(oldKeys));
}

// main function that scan keys
void scanKeysTask(void * params) {
      const TickType_t xFrequency = 20/portTICK_PERIOD_MS;
      TickType_t xLastWakeTime = xTaskGetTickCount();
      uint8_t TX_Message[8] = {0}; // Stores outgoing messages on the CAN Bus

      while (1) {

            // Array of piano keys which are pressed
            boardkeys_t keyPressed;
            
            // Check the state of each key
            for (int i = 0;i < 7; i++){
                  setRow(i);
                  delay_microseconds(4);
                  uint8_t keys = readCols();

                  // Roll previous code into a loop
                  for (int j=0; j< 4; j++){
                        uint8_t bitmask = 1 << j;
                        uint8_t bitShiftedResult = (keys & bitmask) >> j;
                        keyPressed[i*4 + j] = bitShiftedResult;
                  }

            }

            //Update which notes are being played
            uint8_t tmpOctave = __atomic_load_n(&octave,__ATOMIC_RELAXED);
            for(int i=0; i<12; i++){
                  __atomic_store_n(&playedNotes[12*tmpOctave + i],!keyPressed[i],__ATOMIC_RELAXED);
            }

            // Decode whether any of the knobs are being turned
            // ! And if key state has changed since previous iteration
            knobDecode(keyPressed, TX_Message);

            /*
            // ~ debug code
            lcd_t lcd;
            sprintf(lcd, "%c %u %u", TX_Message[0], TX_Message[1], TX_Message[2]);
            DEBUG_PRINT("Read Pins");

            u8g2_ClearBuffer(&u8g2);
            u8g2_SetFont(&u8g2, u8g2_font_ncenB08_tr);
            u8g2_DrawStr(&u8g2, 2, 10, lcd);  // write something to the internal memory
            u8g2_SendBuffer(&u8g2);
            // ~ end debug code
            */

            vTaskDelayUntil( &xLastWakeTime, xFrequency );
      }
}

void init_keydetect() {
      DEBUG_PRINT("Initialising Detect Keys");
      if (xTaskCreate(scanKeysTask, "Detect keys", 128, NULL, 2, NULL) != pdPASS) {
            DEBUG_PRINT("ERROR");
            print(xPortGetFreeHeapSize());
      }
}