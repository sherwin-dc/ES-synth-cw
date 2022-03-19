#include "usart.h"
#include <stdlib.h>
#include "tim.h"

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG_PRINTS


#define DEBUG_PRINT(X)                                                    \
HAL_UART_Transmit(&hlpuart1, (uint8_t *)__func__ , sizeof(__func__), 100); \
HAL_UART_Transmit(&hlpuart1, (uint8_t *) X "\n", sizeof(X), 100);     


#define DEBUG_PRINT_INT(X)                                                 \
HAL_UART_Transmit(&hlpuart1, (uint8_t *)__func__ , sizeof(__func__), 100); \
HAL_UART_Transmit(&hlpuart1, (uint8_t *) (itoa(X) + "\n"), sizeof(X), 100);    

#else

#define DEBUG_PRINT(X)
#define DEBUG_PRINT_INT(X)

#endif

#define START_TIMING   \
DEBUG_PRINT("Start Timing"); \
__HAL_TIM_SET_COUNTER(&htim6,0);  

#define END_TIMING  \
print(__HAL_TIM_GET_COUNTER(&htim6)); \
DEBUG_PRINT("microseconds. End Timing");

void print(uint32_t val);

void blink_LED_error();

#ifdef __cplusplus
}
#endif