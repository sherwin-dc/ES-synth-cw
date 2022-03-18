#include <string>
#include "lcd.h"
#include "main.h"
#include <cstring>
#include <vector>

u8g2_t u8g2;
 QueueHandle_t lcdQueue;

uint8_t u8x8_byte_hw_i2c(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
  static uint8_t buffer[32];		/* u8g2/u8x8 will never send more than 32 bytes between START_TRANSFER and END_TRANSFER */
  static uint8_t buf_idx;
  uint8_t *data;
 
  switch(msg)
  {
    case U8X8_MSG_BYTE_SEND:
      data = (uint8_t *)arg_ptr;      
      while( arg_int > 0 ) {
          buffer[buf_idx++] = *data;
          data++;
          arg_int--;
      }      
      break;
    case U8X8_MSG_BYTE_INIT:
      /* init done in main */
      break;
    case U8X8_MSG_BYTE_SET_DC:
      /* ignored for i2c */
      break;
    case U8X8_MSG_BYTE_START_TRANSFER:
      buf_idx = 0;
      break;
    case U8X8_MSG_BYTE_END_TRANSFER:
      HAL_I2C_Master_Transmit(&hi2c1, u8x8_GetI2CAddress(u8x8), buffer, buf_idx, 1000);
      break;
    default:
      return 0;
  }
  return 1;
}

uint8_t u8x8_gpio_and_delay(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8
      // HAL initialisation done elsewhere
      break;							// can be used to setup pins
    case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
      break;    
    case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
      __NOP();
      break;
    case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
      for (uint16_t n = 0; n < 320; n++)
      {
        __NOP();
      }
      break;
    case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
      HAL_Delay(arg_int);
      break;
    case U8X8_MSG_DELAY_I2C:				// arg_int is the I2C speed in 100KHz, e.g. 4 = 400 KHz
      delay_microseconds(5);
      break;							// arg_int=1: delay by 5us, arg_int = 4: delay by 1.25us
    case U8X8_MSG_GPIO_D0:				// D0 or SPI clock pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_CLOCK:
      break;
    case U8X8_MSG_GPIO_D1:				// D1 or SPI data pin: Output level in arg_int
    //case U8X8_MSG_GPIO_SPI_DATA:
      break;
    case U8X8_MSG_GPIO_D2:				// D2 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D3:				// D3 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D4:				// D4 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D5:				// D5 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D6:				// D6 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_D7:				// D7 pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_CS2:				// CS2 (chip select) pin: Output level in arg_int
      break;
    case U8X8_MSG_GPIO_I2C_CLOCK:		// arg_int=0: Output low at I2C clock pin
      break;							// arg_int=1: Input dir with pullup high for I2C clock pin
    case U8X8_MSG_GPIO_I2C_DATA:			// arg_int=0: Output low at I2C data pin
      break;							// arg_int=1: Input dir with pullup high for I2C data pin
    case U8X8_MSG_GPIO_MENU_SELECT:
      u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_NEXT:
      u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_PREV:
      u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_HOME:
      u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;

  }
  return 1;
}



void update_lcd(void * params) {

  const TickType_t xFrequency = 100/portTICK_PERIOD_MS;
  TickType_t xLastWakeTime = xTaskGetTickCount();

  // maybe this can be global somewhere?
  std::vector<std::string> notes = {"C","C#","D","D#","E","F","F#","G","G#","A","A#","B"};
  std::vector<std::string> sounds = {"SAW","POLY","CHORUS","LASER","SINE","5","6","7","8","9"};

  while (1) {
    // START_TIMING
    
    // CAN Bus stuff
    // uint32_t CAN_RX_ID;
    // Poll for recieved messages
    // while (CAN_CheckRXLevel()) {
    //   CAN_RX( &CAN_RX_ID, RX_Message );
    // }

    u8g2_ClearBuffer(&u8g2); // Clear content on screen
    u8g2_SetFont(&u8g2, u8g2_font_smallsimple_tr); // Set font size

    // Variable used to position data on screen
    uint8_t tmpOffset = __atomic_load_n(&screenOffset,__ATOMIC_RELAXED);

    char tmp0 [40] = "NOTES: ";
    for(int i=0; i < 9*12; i++){
      if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)){
        char tmpNote[1024];
        strcpy(tmpNote, notes[i%12].c_str());
        strcat(tmp0,tmpNote);
        tmp0[strlen(tmp0)] = uint8_t(i/12) + 48;
        tmp0[strlen(tmp0)] = '\0';
        strcat(tmp0," ");
      }
    }

    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_DrawStr(&u8g2, 2, 7 - tmpOffset, tmp0);  // write string to display

    // Write out the volume
    char tmp1 [30] = "1. VOLUME: ";
    tmp1[11] = __atomic_load_n(&volume,__ATOMIC_RELAXED) + 48;
    u8g2_DrawStr(&u8g2, 2, 15 - tmpOffset, tmp1);  // write string to display

    // Write out the octave
    char tmp2 [30] = "2. OCTAVE: ";
    tmp2[11] = __atomic_load_n(&octave,__ATOMIC_RELAXED) + 48;
    u8g2_DrawStr(&u8g2, 75, 15 - tmpOffset, tmp2);  // write string to display

    // Write out the sound
    char tmp3 [30] = "3. SOUND: ";
    strcat(tmp3,sounds[__atomic_load_n(&sound,__ATOMIC_RELAXED)].c_str());
    u8g2_DrawStr(&u8g2, 2, 23 - tmpOffset, tmp3);  // write string to display

    // Write out the reverb
    char tmp4 [30] = "4. REVERB: ";
    tmp4[11] = __atomic_load_n(&reverb,__ATOMIC_RELAXED) + 48;
    u8g2_DrawStr(&u8g2, 75, 23 - tmpOffset, tmp4);  // write string to display

    // Draw out menu screen on bottom
    u8g2_DrawStr(&u8g2, 2, 31 - tmpOffset, "1");
    u8g2_DrawStr(&u8g2, 42, 31 - tmpOffset, "2");
    u8g2_DrawStr(&u8g2, 82, 31 - tmpOffset, "3");
    u8g2_DrawStr(&u8g2, 122, 31 - tmpOffset, "4");



    // Print out CAN Rx Buffer
    u8g2_DrawStr(&u8g2, 70, 7, (char *)RX_Message);
    // ? Something funky going on with the queue here.
    // DEBUG_PRINT("!");
    // print( uxQueueMessagesWaiting( msgInQ ) );
    // print( uxQueueSpacesAvailable( msgInQ ) );


    // Send the buffer to the LCD
    u8g2_SendBuffer(&u8g2);

    //DEBUG_PRINT("LCD running")

    // Toggle MCU LED
    HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);

    // END_TIMING

    vTaskDelayUntil( &xLastWakeTime, xFrequency );
  }

}


void init_lcd() {

  u8g2_Setup_ssd1305_i2c_128x32_noname_f(&u8g2, U8G2_R0, u8x8_byte_hw_i2c, u8x8_gpio_and_delay);

  //Initialise display
  setOutMuxBit(4, false);  //Assert display logic reset
  delay_microseconds(2);
  setOutMuxBit(4, true);  //Release display logic reset
  setOutMuxBit(3, true);  //Enable display power supply

  u8g2_InitDisplay(&u8g2); // send init sequence to the display, display is in sleep mode after this,
  u8g2_SetPowerSave(&u8g2, 0); // wake up display*/

}



void start_lcd_thread() {

  DEBUG_PRINT("Initialising Refresh LCD");
  if (xTaskCreate(update_lcd, "Refresh LCD", 128, NULL, 1, NULL) != pdPASS ) {
    DEBUG_PRINT("Error. Free memory: ");
    print(xPortGetFreeHeapSize());
  }

}