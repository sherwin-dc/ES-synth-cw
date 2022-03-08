#include "gpio.h"

extern "C" void setOutMuxBit(const uint8_t bitIdx, const bool value);

extern "C" uint8_t readCols();
extern "C" void setRow(uint8_t rowIdx);

extern "C" void keymatMain();