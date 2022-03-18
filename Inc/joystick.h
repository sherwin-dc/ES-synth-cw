#include "gpio.h"
#include "queue.h"

#ifdef __cplusplus
extern "C" {
#endif

void readJoystick(void * params);
void init_joystick();
int8_t normaliseJoystick(uint32_t value, uint32_t min, uint32_t mid, uint32_t max);

#ifdef __cplusplus
}
#endif