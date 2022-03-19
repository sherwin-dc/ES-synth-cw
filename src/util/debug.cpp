#include "usart.h"
#include <string>
 

extern "C" void print(uint32_t val) {
  std::string str = std::to_string(val) + "\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t *) str.c_str(), str.size(), 100); 
}  

void print(std::string str) {
  str += "\n";
  HAL_UART_Transmit(&hlpuart1, (uint8_t *) str.c_str(), str.size(), 100); 
}  

// REMEMBER to Comment out #define configCHECK_FOR_STACK_OVERFLOW 2
// in FreeRTOSConfig.h as stack checking slows down the program
extern "C" void vApplicationStackOverflowHook( TaskHandle_t *pxTask, 
 signed char *pcTaskName ) {
     DEBUG_PRINT("STACK OVERFLOW in");
     HAL_UART_Transmit(&hlpuart1, (uint8_t*)pcTaskName, 16, 100);
     while (1) {}
 }




