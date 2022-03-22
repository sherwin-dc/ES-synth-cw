#include "usart.h"
#include <string>
 

extern "C" void print(uint32_t val) {
  std::string str = std::to_string(val) + "\n";
  HAL_UART_Transmit(&huart2, (uint8_t *) str.c_str(), str.size(), 100); 
}  

void print(std::string str) {
  str += "\n";
  HAL_UART_Transmit(&huart2, (uint8_t *) str.c_str(), str.size(), 100); 
}  

// REMEMBER to Comment out #define configCHECK_FOR_STACK_OVERFLOW 2
// in FreeRTOSConfig.h as stack checking slows down the program
extern "C" void vApplicationStackOverflowHook( TaskHandle_t *pxTask, 
 signed char *pcTaskName ) {
     DEBUG_PRINT("STACK OVERFLOW in");
     HAL_UART_Transmit(&huart2, (uint8_t*)pcTaskName, 16, 100);
     while (1) {}
 }

extern "C" void CONFIGURE_TIMER_FOR_RUN_TIME_STATS() {
  __HAL_TIM_SET_COUNTER(&htim2,0);
}

extern "C" uint32_t GET_RUN_TIME_COUNTER_VALUE() {
  return __HAL_TIM_GET_COUNTER(&htim2);
}

// Part copied from task.h/task.cpp
void getRunTimeStats(void* params) {
  const TickType_t xFrequency = 1000/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  TaskStatus_t *pxTaskStatusArray;
	volatile UBaseType_t uxArraySize, x;
	uint32_t ulTotalRunTime;

  
  while(1) {
    uxArraySize = uxTaskGetNumberOfTasks();
    pxTaskStatusArray = (TaskStatus_t *) pvPortMalloc( uxArraySize * sizeof( TaskStatus_t ) ); /*lint !e9079 All values returned by pvPortMalloc() have at least the alignment required by the MCU's stack and this allocation allocates a struct that has the alignment requirements of a pointer. */
    if (pxTaskStatusArray != NULL) {
        uxArraySize = uxTaskGetSystemState( pxTaskStatusArray, uxArraySize, &ulTotalRunTime );
        DEBUG_PRINT("Total run time (0.5 ms):");
        print(ulTotalRunTime);

        /* Create a human readable table from the binary data. */
          for( x = 0; x < uxArraySize; x++ )
          {
              const char *pcTaskName = pxTaskStatusArray[x].pcTaskName;	
              uint32_t ulRunTimeCounter = pxTaskStatusArray[x].ulRunTimeCounter;
              configSTACK_DEPTH_TYPE usStackHighWaterMark = pxTaskStatusArray[x].usStackHighWaterMark;	/* The minimum amount of stack space that has remained for the task since the task was created.  The closer this value is to zero the closer the task has come to overflowing its stack. */


              HAL_UART_Transmit(&huart2, (uint8_t *) pcTaskName, 16, 100);
              print(ulRunTimeCounter);
              print(usStackHighWaterMark);
          }

        vPortFree( pxTaskStatusArray );
    }
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }
  
}

extern "C" void init_run_time_stats() {

    if (xTaskCreate(getRunTimeStats, "Runtime stats", 256+128, NULL, 2, NULL) != pdPASS) {
        DEBUG_PRINT("ERROR");
        print(xPortGetFreeHeapSize());
    }
}






