#include <stdint.h>
#include <stdbool.h>

//Initialise the CAN module
uint32_t CAN_Init(bool loopback);

//Enable the CAN module
uint32_t CAN_Start();

//Set up a recevie filter
//Defaults to receive everything
uint32_t setCANFilter(uint32_t filterID, uint32_t maskID, uint32_t filterBank);

//Send a message
uint32_t CAN_TX(uint32_t ID, uint8_t data[8]);

//Get the number of received messages
uint32_t CAN_CheckRXLevel();

//Get a received message from the FIFO
uint32_t CAN_RX(uint32_t &ID, uint8_t data[8]);

//Set up an interrupt on received messages
uint32_t CAN_RegisterRX_ISR(void(& callback)());

//Set up an interrupt on transmitted messages
uint32_t CAN_RegisterTX_ISR(void(& callback)());