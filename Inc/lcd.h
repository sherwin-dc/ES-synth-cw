#include "u8g2.h"
#include "i2c.h"
#include "keymat.h"
#include "delay.h"
#include "task.h"
#include "keymat.h"

#define LCD_LENGTH 20

#ifdef __cplusplus
extern "C" {
#endif

extern u8g2_t u8g2;

typedef char lcd_t[LCD_LENGTH];
extern QueueHandle_t lcdQueue;

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);

void update_lcd(void * params);
void init_lcd();
void start_lcd_thread();
void redraw_lcd(bool master);


#define W_KEY_WIDTH 11
#define W_KEY_STRIDE (W_KEY_WIDTH-1)
#define W_KEY_HEIGHT 20
#define KEYBOARD_START_X 2
#define KEYBOARD_START_Y 1
#define B_KEY_Y (KEYBOARD_START_Y+4)
#define B_KEY_WIDTH 9
#define B_KEY_HEIGHT 7
#define B_KEY_START_X (KEYBOARD_START_X+6)
#define W_KEY_PRESS_Y (KEYBOARD_START_Y+W_KEY_HEIGHT-3)
#define B_KEY_PRESS_Y (B_KEY_Y+B_KEY_HEIGHT-2)
#define KEY_PRES_RAD 1
#define KEY_FONT_OFFSET_W 4
#define KEY_FONT_OFFSET_B 0

#define DRAW_WHITE_KEY(X) \
u8g2_DrawFrame(&u8g2, KEYBOARD_START_X + X * W_KEY_STRIDE, KEYBOARD_START_Y, W_KEY_WIDTH, W_KEY_HEIGHT);

#define DRAW_BLACK_KEY(X) \
u8g2_DrawBox(&u8g2, B_KEY_START_X + X * W_KEY_STRIDE, B_KEY_Y, B_KEY_WIDTH, B_KEY_HEIGHT);

#define DRAW_WHITE_PRESS(X, STR) \
u8g2_DrawStr(&u8g2, KEYBOARD_START_X + X * W_KEY_STRIDE + KEY_FONT_OFFSET_W, W_KEY_PRESS_Y, STR);

#define DRAW_BLACK_PRESS(X, STR) \
u8g2_DrawStr(&u8g2, B_KEY_START_X + X * W_KEY_STRIDE + KEY_FONT_OFFSET_B, B_KEY_PRESS_Y, STR); \
u8g2_DrawStr(&u8g2, B_KEY_START_X + X * W_KEY_STRIDE + 4, B_KEY_PRESS_Y, "#");

#ifdef __cplusplus
}
#endif