#include "gpio.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keymat.h"
#include "Knob.hpp"

// for debug only
#include "lcd.h"
#include <stdio.h>

#include "main.h"

#include <cstring> // Contains the memcpy function

QueueHandle_t boardkeys;
QueueHandle_t boardknobs; 

// this is a local variable in the cpp
Knob knobs[4];

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
// can we change the fucntion name?
void knobDecode(boardkeys_t newKeys) {

      uint8_t TX_Message[8] = {0}; // Stores outgoing messages on the CAN Bus

      // Static variable holding the keys from the last time the function was called
      static boardkeys_t oldKeys = {0};

      for (int i=0; i<4; i++){
            uint8_t oldState = (oldKeys[12+i*2]<<1) + (oldKeys[13+i*2]); // Calculate previous state
            uint8_t newState = (newKeys[12+i*2]<<1) + (newKeys[13+i*2]); // Calculate current state

            // udpate global parameters using the old and new states
            knobs[i].update(oldState,newState);
      }

      // Check for changes in key presses
      uint8_t anyKeyPressed = false;
      for (int i=0; i<12; i++){
            uint8_t oldKeyState = oldKeys[i];
            uint8_t newKeyState = newKeys[i];

            // button state has changed
            if (oldKeyState ^ newKeyState) {
                  anyKeyPressed = true;
                  // TX_Message[1] = i;                  
                  // TX_Message[2] = octave;                  
                  TX_Message[1] = i + 48;
                  TX_Message[2] = octave + 48;

                  // Button has been pressed
                  if (oldKeyState==1)      { TX_Message[0] = 'P'; }
                  // Button has been released
                  else if (oldKeyState==0) { TX_Message[0] = 'R'; }

                  // DEBUG_PRINT("TX CAN MSG");
                  // CAN_TX(0x123, TX_Message);      // Transmit message over CAN
                  //! This queue seems to block indefinitely sometimes
                  //! because transmitCANMessages does not seem to be running
                  // xQueueSend(msgOutQ, TX_Message, portMAX_DELAY);
            } 
      }
      
      // Clear tx buffer
      if (!anyKeyPressed) {
            TX_Message[0] = ' ';
            TX_Message[1] = ' ';
            TX_Message[2] = ' ';
      }

      // Copy newKeys into oldKeys
      memcpy(oldKeys,newKeys,sizeof(oldKeys));
}

// main function that scan keys
void scanKeysTask(void * params) {
      const TickType_t xFrequency = 20/portTICK_PERIOD_MS;
      TickType_t xLastWakeTime = xTaskGetTickCount();

      while (1) {
            // DEBUG_PRINT("1");

            // Array of piano keys which are pressed
            boardkeys_t keyPressed;
            
            // Check the state of each key
            for (int i = 0;i < 7; i++){
                  setRow(i);
                  delay_microseconds(4);
                  uint8_t keys = readCols();

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

            // update recording state
            if (!keyPressed[20]) {
              __atomic_store_n(&isRecording, 1, __ATOMIC_RELAXED);
            }
            
            if (!keyPressed[21]) {
              __atomic_store_n(&isRecording, 0, __ATOMIC_RELAXED);
            }
            

            // Decode whether any of the knobs are being turned
            // ! And if key state has changed since previous iteration
            knobDecode(keyPressed);

            // DEBUG_PRINT("2");

            vTaskDelayUntil( &xLastWakeTime, xFrequency );
      }
}

void init_keydetect() {
      DEBUG_PRINT("Initialising Detect Keys");
      if (xTaskCreate(scanKeysTask, "Detect keys", 512, NULL, 7, NULL) != pdPASS) {
            DEBUG_PRINT("ERROR");
            print(xPortGetFreeHeapSize());
      }

      // init knob array with corresponding parameters
      knobs[0] = Knob(&reverb);
      knobs[1] = Knob(&sound);
      knobs[2] = Knob(&octave);
      knobs[3] = Knob(&volume);
}

