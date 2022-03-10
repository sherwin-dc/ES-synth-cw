#include "u8g2.h"
#include "i2c.h"
#include "keymat.h"
#include "delay.h"
#include "task.h"
#include "keymat.h"

#ifdef __cplusplus
extern "C" {
#endif

extern u8g2_t u8g2;

typedef char lcd_t[20];
extern QueueHandle_t lcdQueue;

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void update_lcd(void * params);
void init_lcd();

#ifdef __cplusplus
}
#endif