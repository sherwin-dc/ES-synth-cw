#include "gpio.h"

#ifdef __cplusplus
extern "C" {
#endif

void setOutMuxBit(const uint8_t bitIdx, const int value);

uint8_t readCols();
void setRow(uint8_t rowIdx);

void keymatMain();

#ifdef __cplusplus
}
#endif