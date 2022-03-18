#include "gpio.h"
#include "queue.h"
#include "can_comms.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint8_t boardkeys_t[28];
extern QueueHandle_t boardkeys;

typedef int8_t knob_t[4];
extern QueueHandle_t boardknobs;

void setOutMuxBit(const uint8_t bitIdx, const int value);

uint8_t readCols();
void setRow(uint8_t rowIdx);
void knobDecode(boardkeys_t newKeys, uint8_t* TX_Message_Ptr);

void scanKeysTask(void * params);
void init_keydetect();

#ifdef __cplusplus
}
#endif