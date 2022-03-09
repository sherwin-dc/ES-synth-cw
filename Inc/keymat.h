#include "gpio.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef char boardkeys_t[28];
extern QueueHandle_t boardkeys;

void setOutMuxBit(const uint8_t bitIdx, const int value);

uint8_t readCols();
void setRow(uint8_t rowIdx);

void keymatMain();

#ifdef __cplusplus
}
#endif