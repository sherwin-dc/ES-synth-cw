#include "gpio.h"
#include "delay.h"

extern "C" void setOutMuxBit(const uint8_t bitIdx, const bool value) {
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
extern "C" uint8_t readCols() {
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
extern "C" void setRow(uint8_t rowIdx) {
      if (rowIdx > 7) {
            // error
            return;
      }

      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_0_GPIO_Port, Row_Sel_0_Pin, (rowIdx & 0x01) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_1_GPIO_Port, Row_Sel_1_Pin, (rowIdx & 0x02) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_2_GPIO_Port, Row_Sel_2_Pin, (rowIdx & 0x04) > 0 ? GPIO_PIN_SET : GPIO_PIN_RESET);
      HAL_GPIO_WritePin(Row_Sel_En_GPIO_Port, Row_Sel_En_Pin, GPIO_PIN_SET);

}