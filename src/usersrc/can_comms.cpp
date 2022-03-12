/* Source code to handle CAN Bus communications */
#include "gpio.h"
#include "delay.h"
#include "FreeRTOS.h"
#include "task.h"
#include "keymat.h"

// for debug only
#include "lcd.h"
#include <stdio.h>

#include "main.h"

#include <cstring> // Contains the memcpy function
#include <algorithm> // Contains max and min functions

// Code copied from Example. Doesn't work yet.
/*
void CAN_RX_ISR (void) {
    uint8_t RX_Message_ISR[8];
    uint32_t ID;
    CAN_RX(ID, RX_Message_ISR);
    xQueueSendFromISR(msgInQ, RX_Message_ISR, NULL);
}
*/