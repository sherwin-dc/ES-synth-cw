/* Source code to handle CAN Bus communications */
#include "gpio.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keymat.h"
#include "can_comms.h"
#include "can.h"
#include "main.h"

// for debug only
#include "lcd.h"
#include <stdio.h>


#include <cstring> // Contains the memcpy function
#include <algorithm> // Contains max and min functions

//Overwrite the weak default IRQ Handlers and callabcks
// extern "C" void CAN1_RX0_IRQHandler(void);
// extern "C" void CAN1_TX_IRQHandler(void);

#ifdef __cplusplus
extern "C" {
#endif


// Must match the declaration in the header file
#ifdef __cplusplus
    //Pointer to user ISRS
    // extern "C" void (*CAN_RX_ISR_PTR)() = NULL;
    // extern "C" void (*CAN_TX_ISR_PTR)() = NULL;
    void (*CAN_RX_ISR_PTR)() = NULL;
    void (*CAN_TX_ISR_PTR)() = NULL;

#else
    void (*CAN_RX_ISR_PTR)() = NULL;
    void (*CAN_TX_ISR_PTR)() = NULL;
#endif

/*
//Initialise CAN dependencies: GPIO and clock
void HAL_CAN_MspInit(CAN_HandleTypeDef* hcan1) {

  //Set up the pin initialisation
  GPIO_InitTypeDef GPIO_InitCAN_TX = {
    GPIO_PIN_12,            //PA12 is CAN TX
    GPIO_MODE_AF_PP,        //Alternate function, push-pull driver
    GPIO_NOPULL,            //No pull-up
    GPIO_SPEED_FREQ_MEDIUM, //Medium slew rate
    GPIO_AF9_CAN1           //Alternate function is CAN
    };

  GPIO_InitTypeDef GPIO_InitCAN_RX = {
    GPIO_PIN_11,            //PA11 is CAN RX
    GPIO_MODE_AF_PP,        //Alternate function, push-pull driver
    GPIO_PULLUP,            //Pull-up enabled
    GPIO_SPEED_FREQ_MEDIUM, //Medium slew rate
    GPIO_AF9_CAN1           //Alternate function is CAN    
    };

  //Enable the CAN and GPIO clocks
  __HAL_RCC_CAN1_CLK_ENABLE(); //Enable the CAN interface clock
  __HAL_RCC_GPIOA_CLK_ENABLE(); //Enable the clock for the CAN GPIOs

  //Initialise the pins
  HAL_GPIO_Init(GPIOA, &GPIO_InitCAN_TX); //Configure CAN pin
  HAL_GPIO_Init(GPIOA, &GPIO_InitCAN_RX); //Configure CAN pin
}
*/

uint32_t CAN_Init(bool loopback) {
  if (loopback)
    hcan1.Init.Mode = CAN_MODE_LOOPBACK;
  return (uint32_t) HAL_CAN_Init(&hcan1);
}


uint32_t setCANFilter(uint32_t filterID, uint32_t maskID, uint32_t filterBank) {

  //Set up the filter definition
  CAN_FilterTypeDef filterInfo = {
    (filterID << 5) & 0xffe0,   //Filter ID
    0,                          //Filter ID LSBs = 0
    (maskID << 5) & 0xffe0,     //Mask MSBs
    0,                          //Mask LSBs = 0
    0,                          //FIFO selection
    filterBank & 0xf,           //Filter bank selection
    CAN_FILTERMODE_IDMASK,      //Mask mode
    CAN_FILTERSCALE_32BIT,      //32 bit IDs
    CAN_FILTER_ENABLE,          //Enable filter
    0                           //uint32_t SlaveStartFilterBank
  };

  return (uint32_t) HAL_CAN_ConfigFilter(&hcan1, &filterInfo);
}


uint32_t CAN_Start() {
  return (uint32_t) HAL_CAN_Start(&hcan1);
}


uint32_t CAN_TX(uint32_t ID, uint8_t data[8]) {

  //Set up the message header
  CAN_TxHeaderTypeDef txHeader = {
    ID & 0x7ff,                 //Standard ID
    0,                          //Ext ID = 0
    CAN_ID_STD,                 //Use Standard ID
    CAN_RTR_DATA,               //Data Frame
    8,                          //Send 8 bytes
    DISABLE                     //No time triggered mode
  };

  //Wait for free mailbox
  while (!HAL_CAN_GetTxMailboxesFreeLevel(&hcan1));

  //Start the transmission
  return (uint32_t) HAL_CAN_AddTxMessage(&hcan1, &txHeader, data, NULL);
}


uint32_t CAN_CheckRXLevel() {
  return HAL_CAN_GetRxFifoFillLevel(&hcan1, 0);
}


uint32_t CAN_RX(uint32_t *ID, uint8_t data[8]) {
  CAN_RxHeaderTypeDef rxHeader;

  //Wait for message in FIFO
  while (!HAL_CAN_GetRxFifoFillLevel(&hcan1, 0));
  
  //Get the message from the FIFO
  uint32_t result = (uint32_t) HAL_CAN_GetRxMessage(&hcan1, 0, &rxHeader, data);

  //Store the ID from the header
  *ID = rxHeader.StdId;

  return result;
}

uint32_t CAN_RegisterRX_ISR(void(*callback)()) {
  //Store pointer to user ISR
  CAN_RX_ISR_PTR = callback;

  //Enable message received interrupt in HAL
  uint32_t status = (uint32_t) HAL_CAN_ActivateNotification (&hcan1, CAN_IT_RX_FIFO0_MSG_PENDING);

  //Switch on the interrupt
//   HAL_NVIC_SetPriority (CAN1_RX0_IRQn, 5, 0);
//   HAL_NVIC_EnableIRQ (CAN1_RX0_IRQn);

  return status;
}

uint32_t CAN_RegisterTX_ISR(void(*callback)()) {
  //Store pointer to user ISR
  CAN_TX_ISR_PTR = callback;

  //Enable message received interrupt in HAL
  uint32_t status = (uint32_t) HAL_CAN_ActivateNotification (&hcan1, CAN_IT_TX_MAILBOX_EMPTY);

  //Switch on the interrupt
//   HAL_NVIC_SetPriority (CAN1_TX_IRQn, 5, 0);
//   HAL_NVIC_EnableIRQ (CAN1_TX_IRQn);

  return status;
}

void HAL_CAN_RxFifo0MsgPendingCallback (CAN_HandleTypeDef * hcan){
    //DEBUG_PRINT("CAN_RX_IRQ");
    // Call the user ISR if it has been registered
    if (CAN_RX_ISR_PTR)
        CAN_RX_ISR_PTR();
}

void HAL_CAN_TxMailbox0CompleteCallback (CAN_HandleTypeDef * hcan){
    //DEBUG_PRINT("CAN_TX_IRQ");
    //Call the user ISR if it has been registered
    if (CAN_TX_ISR_PTR)
        CAN_TX_ISR_PTR();
}

void HAL_CAN_TxMailbox1CompleteCallback (CAN_HandleTypeDef * hcan){
    //DEBUG_PRINT("CAN_TX_IRQ");
    //Call the user ISR if it has been registered
    if (CAN_TX_ISR_PTR)
        CAN_TX_ISR_PTR();
}

void HAL_CAN_TxMailbox2CompleteCallback (CAN_HandleTypeDef * hcan){
    //DEBUG_PRINT("CAN_TX_IRQ");
    //Call the user ISR if it has been registered
    if (CAN_TX_ISR_PTR)
        CAN_TX_ISR_PTR();
}

void CAN_RX_ISR() {
    uint8_t RX_Message_ISR[8];
    uint32_t ID;
    uint32_t rx_res = CAN_RX(&ID, RX_Message_ISR);
    BaseType_t queueSendRes = pdTRUE; // Placeholder
    // ? xQueueSendFromISR hangs the program for some reason
    // queueSendRes = xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
    print(queueSendRes);
}


// Create a decode thread to handle incoming CAN messages
void decodeCANMessages(void *params) {


    // THESE TWO LINES DO NOT NEED TO BE HERE; ADDED BY EDVARD TO STOP TASK FROM TAKING UP CPU TIME
    const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    //


    // //DEBUG_PRINT("~");
    while(1){
      ////DEBUG_PRINT("!");
      BaseType_t queueReceiveRes = pdPASS;  // Placeholder
      // ? xQueueReceive also blocks the program.
      // ? Try toggling between portMAX_DELAY (blocks forever) and (TickType_t)0 (no blocking) for testing
      // queueReceiveRes = xQueueReceive(msgInQ, RX_Message, (TickType_t)0);
      if ( queueReceiveRes == pdPASS ){
          ////DEBUG_PRINT("can rx smth");
          // TODO decode thread in response to incoming messages

      } else {
          // //DEBUG_PRINT("x");
      }


    // THIS LINE DOes NOT NEED TO BE HERE; ADDED BY EDVARD TO STOP TASK FROM TAKING UP CPU TIME
    vTaskDelayUntil( &xLastWakeTime, xFrequency );
    //


    }

    ////DEBUG_PRINT("decodeCANMessages exited!");
    vTaskDelete(NULL);

}

void init_can_rx_decode(){
    //DEBUG_PRINT("Initializing CAN RX Decode");
    xQueueReset(msgInQ);    // Ensure message queue is empty
    if (xTaskCreate(decodeCANMessages, "CAN RX ISR", 128, NULL, 1, NULL) != pdPASS) {
        //DEBUG_PRINT("Error. Free memory: ");
        print(xPortGetFreeHeapSize());
    }
}

#ifdef __cplusplus
}
#endif