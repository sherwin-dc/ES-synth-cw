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
  std::vector<std::string> sounds = {"SAW   ","POLY  ","CHORUS","LASER ","SINE  ","5     ","6     ","7     ","8     ","9     "};
  uint8_t recordingBlinking = 0;
  while (1) {
    // START_TIMING
    // DEBUG_PRINT("1");

    u8g2_ClearBuffer(&u8g2); // Clear content on screen
    u8g2_SetFont(&u8g2, u8g2_font_smallsimple_tr); // Set font size

    u8g2_SetDrawColor(&u8g2, 2);
    // draw white keys
    DRAW_WHITE_KEY(0);
    DRAW_WHITE_KEY(1);
    DRAW_WHITE_KEY(2);
    DRAW_WHITE_KEY(3);
    DRAW_WHITE_KEY(4);
    DRAW_WHITE_KEY(5);
    DRAW_WHITE_KEY(6);

    // draw black keys
    DRAW_BLACK_KEY(0);
    DRAW_BLACK_KEY(1);
    DRAW_BLACK_KEY(3);
    DRAW_BLACK_KEY(4);
    DRAW_BLACK_KEY(5);

    // bitbanged array for all notes
    uint16_t pianoKeys[12] = {0};

    // collate notes
    for (int i=0; i<9*12; ++i) {
      if(__atomic_load_n(&playedNotes[i],__ATOMIC_RELAXED)) {
        pianoKeys[i%12] |= 1 << (i/12);
      }
    }

    auto printKeyPress = [&](uint8_t note, uint16_t octave) {
      bool drawCol = false;
      if (octave) drawCol = true;

      switch (note)
      {
      case 0:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(0, "C");
        break;
      case 1:
        u8g2_SetDrawColor(&u8g2, !drawCol);
        DRAW_BLACK_PRESS(0, "C");
        break;
      case 2:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(1, "D");
        break;
      case 3:
        u8g2_SetDrawColor(&u8g2, !drawCol);
        DRAW_BLACK_PRESS(1, "D");
        break;
      case 4:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(2, "E");
        break;
      case 5:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(3, "F");
        break;
      case 6:
        u8g2_SetDrawColor(&u8g2, !drawCol);
        DRAW_BLACK_PRESS(3, "F");
        break;
      case 7:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(4, "G");
        break;
      case 8:
        u8g2_SetDrawColor(&u8g2, !drawCol);
        DRAW_BLACK_PRESS(4, "G");
        break;
      case 9:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(5, "A");
        break;
      case 10:
        u8g2_SetDrawColor(&u8g2, !drawCol);
        DRAW_BLACK_PRESS(5, "A");
        break;
      case 11:
        u8g2_SetDrawColor(&u8g2, drawCol);
        DRAW_WHITE_PRESS(6, "B");
        break;
      
      default:
        break;
      }
    };

    // print key presses
    u8g2_SetFont(&u8g2, u8g2_font_u8glib_4_tf); // Set font size
    u8g2_SetFontMode(&u8g2, 1);
    for (int i=0; i<12; ++i) {
        printKeyPress(i, pianoKeys[i]);
    }

    u8g2_SetFontMode(&u8g2, 0);
    u8g2_SetDrawColor(&u8g2, 1);
    u8g2_SetFont(&u8g2, u8g2_font_5x7_mf);

    // Write out the octave
    char oct [2] = {0};
    oct[0] = __atomic_load_n(&octave,__ATOMIC_RELAXED) + 48;
    u8g2_DrawStr(&u8g2, 50, 32, oct);

    uint8_t tmpIsMaster = __atomic_load_n(&isMaster,__ATOMIC_RELAXED);
    if (tmpIsMaster) {
      // Write out the volume
      char vol [2] = {0};
      vol[0] = __atomic_load_n(&volume,__ATOMIC_RELAXED) + 48;
      u8g2_DrawStr(&u8g2, 10, 32, vol);

      // Write out the waveform
      u8g2_DrawStr(&u8g2, 80, 32, sounds[__atomic_load_n(&sound,__ATOMIC_RELAXED)].c_str());

      // Write out the reverb
      char rev [2] = {0};
      rev[0] = __atomic_load_n(&reverb,__ATOMIC_RELAXED) + 48;
      u8g2_DrawStr(&u8g2, 120, 32, rev);


      // Print out pitch and modulation (from joystick)
      uint32_t pitch = __atomic_load_n(&joystick.pitch ,__ATOMIC_RELAXED);
      u8g2_DrawStr(&u8g2, 86, 7, u8x8_u16toa(pitch/100, 2));
      
      uint32_t modulation = __atomic_load_n(&joystick.modulation,__ATOMIC_RELAXED);
      u8g2_DrawStr(&u8g2, 116, 7, u8x8_u16toa(modulation/100, 2));

      // Draw out recording
      u8g2_SetFontMode(&u8g2, 1);
      unsigned char recordIcon[] = {0xff,0x81,0x99,0xbd,0xbd,0x99,0x81,0xff};

      if(__atomic_load_n(&isRecording, __ATOMIC_RELAXED)) {
        ++recordingBlinking;
        if (recordingBlinking==5) {
          unsigned char currentlyRecordingIcon[] = {0xff,0x81,0x81,0x81,0x81,0x81,0x81,0xff};
          u8g2_DrawXBM(&u8g2, 78, 12, 8, 8, currentlyRecordingIcon);
        } else if (recordingBlinking==10) {
          recordingBlinking = 0;
          u8g2_DrawXBM(&u8g2, 78, 12, 8, 8, recordIcon);
        }
        
        unsigned char stopRecordingIcon[] = {0xff,0x81,0xbd,0xbd,0xbd,0xbd,0x81,0xff};
        u8g2_DrawXBM(&u8g2, 116, 12, 8, 8, stopRecordingIcon);
        
      } else {
        u8g2_SetDrawColor(&u8g2, 0);
        u8g2_DrawBox(&u8g2, 116, 12, 8, 8);
        u8g2_SetDrawColor(&u8g2, 1);
        
        u8g2_DrawXBM(&u8g2, 78, 12, 8, 8, recordIcon);
      }
    }
    

    // Print out CAN Rx Buffer
    // u8g2_DrawStr(&u8g2, 70, 7, (char *)RX_Message);

    // Send the buffer to the LCD
    u8g2_SendBuffer(&u8g2);


    // Toggle MCU LED
    HAL_GPIO_TogglePin(GPIOB, LED_BUILTIN_Pin);


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

  u8g2_ClearBuffer(&u8g2);
  // u8g2_SetDrawColor(&u8g2, 2);
  // // draw white keys
  // DRAW_WHITE_KEY(0);
  // DRAW_WHITE_KEY(1);
  // DRAW_WHITE_KEY(2);
  // DRAW_WHITE_KEY(3);
  // DRAW_WHITE_KEY(4);
  // DRAW_WHITE_KEY(5);
  // DRAW_WHITE_KEY(6);

  // // draw black keys
  // DRAW_BLACK_KEY(0);
  // DRAW_BLACK_KEY(1);
  // DRAW_BLACK_KEY(3);
  // DRAW_BLACK_KEY(4);
  // DRAW_BLACK_KEY(5);


  u8g2_SetDrawColor(&u8g2, 1);
  u8g2_SetFont(&u8g2, u8g2_font_smallsimple_tr); // Set font size
  unsigned char volumeIcon[] = {0xd4,0xe6,0xef,0xef,0xe6,0xd4};
  u8g2_DrawXBM(&u8g2, 2, 26, 6, 6, volumeIcon);

  unsigned char musicNoteIcon[] = {0xfc,0xe6,0xe2,0xe2,0xf3,0xf3};
  u8g2_DrawXBM(&u8g2, 42, 26, 6, 6, musicNoteIcon);

  unsigned char waveIcon[] = {0xc4,0xea,0xd1,0xc0,0xf9,0xe7};
  u8g2_DrawXBM(&u8g2, 72, 26, 6, 6, waveIcon);

  unsigned char reverbIcon[] = {0xc9,0xd2,0xc9,0xd2,0xc9,0xd2};
  u8g2_DrawXBM(&u8g2, 112, 26, 6, 6, reverbIcon);

  unsigned char pitchIcon[] = {0xd5,0xe5,0xe7,0xe2,0xe2,0xd2};
  u8g2_DrawXBM(&u8g2, 78, 1, 6, 6, pitchIcon);

  unsigned char modulationIcon[] = {0xca,0xeb,0xff,0xff,0xeb,0xca};
  u8g2_DrawXBM(&u8g2, 106, 1, 6, 6, modulationIcon);


}



void start_lcd_thread() {

  DEBUG_PRINT("Initialising Refresh LCD");
  if (xTaskCreate(update_lcd, "Refresh LCD", 512, NULL, 1, NULL) != pdPASS ) {
    DEBUG_PRINT("ERROR");
    print(xPortGetFreeHeapSize());
  }

}