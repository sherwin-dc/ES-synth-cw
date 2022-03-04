#include "usart.h"
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define DEBUG_PRINT(X)                                                    \
HAL_UART_Transmit(&hlpuart1, (uint8_t *)__func__ , sizeof(__func__), 100); \
HAL_UART_Transmit(&hlpuart1, (uint8_t *) X "\n", sizeof(X), 100);     


#define DEBUG_PRINT_INT(X)                                                    \
HAL_UART_Transmit(&hlpuart1, (uint8_t *)__func__ , sizeof(__func__), 100); \
HAL_UART_Transmit(&hlpuart1, (uint8_t *) (itoa(X) + "\n"), sizeof(X), 100);     


void print(uint32_t val);

#ifdef __cplusplus
}
#endif