/* 
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ha Thach (tinyusb.org)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */
 
/*
 * MIT License
 *
 * Copyright (c) 2023 Martin Stokroos (ezResponseBox)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
 * copies of the Software, and to permit persons to whom the Software is 
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in 
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "hardware/gpio.h"
#include "pico/util/queue.h"

#include "bsp/board.h"
#include "tusb.h"
#include "usb_descriptors.h"

#define HZ 100  //digital input sampling delay in us.
#define NCHAN 8 //max number of input/output channels = 8
#define FIRST_GPIO_IN 0
#define FIRST_GPIO_OUT (FIRST_GPIO_IN + 8)
#define RANGE_GPIO 0xFFFF
#define KEY_JOY_SEL_PIN 18
#define KEY_MODE_SEL_PIN 19
#define DEBOUNCE_SEL_PIN 20
#define INVERT_OUTPUTS_SEL_PIN 21

//--------------------------------------------------------------------+
// MACRO CONSTANT TYPEDEF PROTYPES
//--------------------------------------------------------------------+

/* Blink pattern
 * - 250 ms  : device not mounted
 * - 1000 ms : device mounted
 * - 2500 ms : device is suspended
 */
enum {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

// prototypes
void led_blinking_task(void);
void hid_task(void);
static void send_hid_report(uint8_t report_id);
bool timer_callback(repeating_timer_t *rt);
void to_hex(uint8_t* in, uint8_t* out);
void to_keycode(uint8_t* in, size_t insz, uint8_t* out);

// globals
static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
static uint32_t portsAll;
static uint8_t newEvent, lastEvent, xMask;
static bool eventUpdate;
typedef struct {
  bool ncContacts;
  bool deviceMode;
  bool keyMode;
  bool debounceOn;
  bool invertOp;
} ezConfig;
ezConfig config;

static uint8_t window[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // store 8-ch parallel window data
static const uint8_t filtered[32] =
{
  0,  //00000
  0,  //00001
  0,  //00010
  1,  //00011
  0,  //00100
  1,  //00101
  1,  //00110
  1,  //00111
  0,  //01000
  1,  //01001
  1,  //01010
  1,  //01011
  1,  //01100
  1,  //01101
  1,  //01110
  1,  //01111
  0,  //10000
  0,  //10001
  0,  //10010
  1,  //10011
  0,  //10100
  1,  //10101
  1,  //10110
  1,  //10111
  0,  //11000
  1,  //11001
  1,  //11010
  1,  //11011
  1,  //11100
  1,  //11101
  1,  //11110
  1   //11111
};


/*------------- MAIN -------------*/
int main(void)
{
  gpio_init(KEY_JOY_SEL_PIN);
  gpio_init(KEY_MODE_SEL_PIN);
  gpio_init(DEBOUNCE_SEL_PIN);
  gpio_init(INVERT_OUTPUTS_SEL_PIN);
  gpio_set_dir(KEY_JOY_SEL_PIN, GPIO_IN);
  gpio_set_dir(KEY_MODE_SEL_PIN, GPIO_IN);
  gpio_set_dir(DEBOUNCE_SEL_PIN, GPIO_IN);
  gpio_set_dir(INVERT_OUTPUTS_SEL_PIN, GPIO_IN);
  gpio_pull_up(KEY_JOY_SEL_PIN);
  gpio_pull_up(KEY_MODE_SEL_PIN);
  gpio_pull_up(DEBOUNCE_SEL_PIN);
  gpio_pull_up(INVERT_OUTPUTS_SEL_PIN);

  // Read the hardware configuration status
  config.deviceMode = gpio_get(KEY_JOY_SEL_PIN);
  config.keyMode = gpio_get(KEY_MODE_SEL_PIN);
  config.debounceOn = gpio_get(DEBOUNCE_SEL_PIN);
  config.invertOp = gpio_get(INVERT_OUTPUTS_SEL_PIN);

  for (int gpio = FIRST_GPIO_IN; gpio < FIRST_GPIO_IN + NCHAN; gpio++)
  {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_IN);
    gpio_pull_up(gpio);
  }

  for (int gpio = FIRST_GPIO_OUT; gpio < FIRST_GPIO_OUT + NCHAN; gpio++)
  {
    gpio_init(gpio);
    gpio_set_dir(gpio, GPIO_OUT);
    if(!config.invertOp) {
      gpio_set_outover(gpio, GPIO_OVERRIDE_INVERT); //invert output channels
    }
  }  

  // Detect if one or more switches are NC and pulling down the input.
  portsAll = ~gpio_get_all(); // Read all gpio's (29-0) at once and bitwise invert.
  portsAll = portsAll >> FIRST_GPIO_IN;
  if ((portsAll & 0xFF) > 0) // Use bitmask for 8 bits. (Size of NCHAN could be different!)
  {
    config.ncContacts = true;
  }

  board_init();
  tusb_init();

  repeating_timer_t timer;
  // negative timeout means exact delay in us (rather than delay between callbacks)
  if (!add_repeating_timer_us(-HZ, timer_callback, NULL, &timer)) {
    //printf("Failed to add timer\n");
    return 1;
  }

  while (1)
  {
    tud_task(); // tinyusb device task
    led_blinking_task();

    hid_task();
    //cancel_repeating_timer(&timer);
  }
}



//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}


//--------------------------------------------------------------------+
// USB HID
//--------------------------------------------------------------------+

// Invoked when sent REPORT successfully to host
// Application can use this to send the next report
// Note: For composite reports, report[0] is report ID
void tud_hid_report_complete_cb(uint8_t instance, uint8_t const* report, uint16_t len)
{
  (void) instance;
  (void) len;
}


// Invoked when received GET_REPORT control request
// Application must fill buffer report's content and return its length.
// Return zero will cause the stack to STALL request
uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen)
{
  // TODO not Implemented
  (void) instance;
  (void) report_id;
  (void) report_type;
  (void) buffer;
  (void) reqlen;

  return 0;
}


// Invoked when received SET_REPORT control request or
// received data on OUT endpoint ( Report ID = 0, Type = 0 )
void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize)
{
  (void) instance;

  if (report_type == HID_REPORT_TYPE_OUTPUT)
  {
    // Set keyboard LED e.g Capslock, Numlock etc...
    if (report_id == REPORT_ID_KEYBOARD)
    {
      // bufsize should be (at least) 1
      if ( bufsize < 1 ) return;

      uint8_t const kbd_leds = buffer[0];

      if (kbd_leds & KEYBOARD_LED_CAPSLOCK)
      {
        // Capslock On: disable blink, turn led on
        blink_interval_ms = 0;
        board_led_write(true);
      } else
      {
        // Caplocks Off: back to normal blink
        board_led_write(false);
        blink_interval_ms = BLINK_MOUNTED;
      }
    }
  }
}



//--------------------------------------------------------------------+
// BLINKING TASK
//--------------------------------------------------------------------+
void led_blinking_task(void)
{
  static uint32_t start_ms = 0;
  static bool led_state = false;

  // blink is disabled
  if (!blink_interval_ms) return;

  // Blink every interval ms
  if ( board_millis() - start_ms < blink_interval_ms) return; // not enough time
  start_ms += blink_interval_ms;

  board_led_write(led_state);
  led_state = 1 - led_state; // toggle
}



//--------------------------------------------------------------------+
// HID TASK
//--------------------------------------------------------------------+
void hid_task(void)
{
  // Remote wakeup
  if ( tud_suspended() && eventUpdate) {
    // Wake up host if we are in suspend mode
    // and REMOTE_WAKEUP feature is enabled by host
    tud_remote_wakeup();
  } else
  {
    if(config.deviceMode == true) {
      send_hid_report(REPORT_ID_KEYBOARD);
    } else {
      send_hid_report(REPORT_ID_GAMEPAD);
    }
  }
}



//--------------------------------------------------------------------+
// SEND HID REPORT
//--------------------------------------------------------------------+
static void send_hid_report(uint8_t report_id)
{
  // skip if hid is not ready yet
  if ( !tud_hid_ready() ) return;


  switch(report_id)
  {
    case REPORT_ID_KEYBOARD:
    {
      // use to avoid send multiple consecutive zero report for keyboard
      static bool has_keyboard_key = false;
      uint8_t keycode[6] = { 0 };

      if ( eventUpdate )
      {
        if(config.keyMode == true) {
          if(lastEvent == 0) { // input changed to zero
            tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
          }
          else
          {
            // Handling multiple (n=6 max) changes at once.
            // In practice, detecting a double key hit will be seldom,
            // because of the high sampling rate. n set to 3.
            uint8_t k, n=0;
            for(k = 0; k < NCHAN; k++) {
              if((lastEvent >> k) & (xMask >> k) & 1) {
                keycode[n] = HID_KEY_1 + k;
                n++;
              }
              if(n > 2) break;
              //keycode[0] = n + 0x1D; // for double hit debugging purpose
            }
          }
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
        } else { // output hex
          enum { hexsz = 2 };
          uint8_t hexcode[hexsz]; // create target array
          to_hex(&lastEvent, hexcode);
          to_keycode(hexcode, hexsz, keycode);
          tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, keycode);
          has_keyboard_key = true;
        }
      eventUpdate = false;
      }
      else {
        // send empty key report if previously has key pressed
        if (has_keyboard_key) tud_hid_keyboard_report(REPORT_ID_KEYBOARD, 0, NULL);
        has_keyboard_key = false;
      }
    }
    break;

    case REPORT_ID_GAMEPAD:
    {
      hid_gamepad_report_t report = {
        .x   = 0, .y = 0, .z = 0,
        .rz = 0, .rx = 0, .ry = 0,
        .hat = 0, 
        .buttons = 0
      };

      if ( eventUpdate ) {
        report.buttons = lastEvent;
        // report.hat = 0 // completing axis data, etc.
        tud_hid_report(REPORT_ID_GAMEPAD, &report, sizeof(report));
        eventUpdate = false;
      }
    }
    break;

    default: break;
  }
}



//--------------------------------------------------------------------+
// Converts a byte to a hexadecimal character string
//
//--------------------------------------------------------------------+
void to_hex(uint8_t* in, uint8_t* out) {
  uint8_t *pin = in;
  const char *hex = "0123456789ABCDEF";
  uint8_t *pout = out;

  *pout++ = hex[(*pin >> 4) & 0xF];
  *pout++ = hex[(*pin++) & 0xF];
  //*pout = 0;
}



//--------------------------------------------------------------------+
// Converts hexadecimal character string to keyboard scan codes
//
//--------------------------------------------------------------------+
void to_keycode(uint8_t* in, size_t insz, uint8_t* out) {
  uint8_t *pin = in;
  uint8_t *pout = out;
  int i = 0;

  for(; i < insz; ++i) {
    if(*pin > 0x40) {  // if ascii letter...
      *pout++ = *pin++ - 0x3D;
    }
    else if(*pin == 0x30) {  // if ascii zero...
      *pout++ = *pin++ - 0x09;
    }
    else {
      *pout++ = *pin++ - 0x13;  // else ascii number...
    }
  }
  *pout = 0;
}






//--------------------------------------------------------------------+
// HARDWARE TIMER ISR
//--------------------------------------------------------------------+
bool timer_callback(repeating_timer_t *rt)
{
  newEvent = 0;

  if(config.ncContacts) {
    portsAll = gpio_get_all(); // Read all gpio's (29-0) at once.
  } else {
    portsAll = ~gpio_get_all(); // Read all gpio's (29-0) at once and bitwise invert.
  }
  portsAll = portsAll >> FIRST_GPIO_IN;

  if(config.debounceOn) {
    // Debounce filter according Steven Pigeon, taken from:
    // https://hbfs.wordpress.com/2008/08/20/debouncing-using-binary-finite-impulse-reponse-filter/
    // window size = 5 bits. Filter delay is two timer periods.
    for(int k = 0; k < NCHAN; k++) {
      newEvent >>= 1; // Right shift the newEvents byte for new empty msb position
      window[k] = ( (window[k] << 1) | ((portsAll >> k) & 1) ) & 0x1f; // calculate the 5 bit window
      newEvent += 128 * filtered[window[k]]; // decide for the new event to become a one or a zero, write the newEvent msb
    }
  } else {
    newEvent = portsAll & 0xFF; // Use bitmask for 8 bits.
  }

  // Do not miss any changes during the USB send.
  // lastEvent is send out, because it does not change during the transfer.
  if(!eventUpdate) {
    xMask = newEvent ^ lastEvent;
    if(xMask > 0) { // Detect any changes and put a flag. 
      lastEvent = newEvent;
      eventUpdate = true;
    }
  }

  // Route debounced events to hardware outputs. Set all GPIOs in one go.
  gpio_put_masked(RANGE_GPIO << FIRST_GPIO_OUT, newEvent << FIRST_GPIO_OUT);
  return true; // keep repeating
}
