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


