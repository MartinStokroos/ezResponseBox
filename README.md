# The ezResponseBox
*An open source Button Response Box for 1-8 buttons based on the Raspberry Pi Pico*

Button response boxes are widely utilized in psychological experiments, typically interfaced as serial devices or as Human Interface Devices (HIDs), such as keyboards or joysticks.

The Raspberry Pi Pico serves as a low-cost microcontroller platform. By incorporating an appropriate number of input buttons (1-8) and a plastic enclosure, a response box can be constructed for under $50, offering specifications comparable to many commercial alternatives. The *ezResponseBox* is compatible with software such as [OpenSesame](https://osdoc.cogsci.nl/), PsychoPy, and E-Prime.

The *ezResponseBox* operates as either a keyboard or joystick-type response box, achieving a typical latency of 1 ms (equivalent to a 1000 Hz update rate). This latency is 10 to 20 times lower than that of standard PC keyboards.

In Keyboard Mode I, the *ezResponseBox* transmits keystrokes corresponding to key numbers 1 to 8 based on the pressed button(s). Simultaneous key presses are communicated within the same USB packet, although this occurrence is rare due to the high input scan rate.

In Keyboard Mode II, the *ezResponseBox* transmits two hexadecimal digits ranging from `00` to `FF`. Applications are responsible for decoding simultaneous key presses.

The advantage of using a joystick HID device lies in its ability to send every state change of the inputs to the host as an eight-bit joystick button state. Decoding of the buttons must be performed within the application program.

The *ezResponseBox* scans eight digital inputs to read the current status of the button knobs. Both Normally Open (NO) and Normally Closed (NC) contacts can be utilized. The type of connected button contacts is determined at power-up. NC contacts facilitate faster detection of response onset. The eight input channels are scanned at a rate of 10 kHz, with readings subjected to debouncing via a FIR digital filter algorithm, incorporating a minimum delay of 200µs which lies within the USB package interval. The debounced information is transmitted in the next available USB packet, with only input state changes sent to the computer.

## Specifications
- USB 2.0 compatible
- works under Windows and Linux
- no drivers needed 
- works as a keyboard or as joystick HID-composite device
- 1ms latency (minimum for HID)
- 10kHz input port scan rate
- integrated switch debouncing filter

GP pin vs. keyboard/joystick button number:

pin: | GP0 | GP1 | GP2 | GP3 | GP4 | GP5 | GP6 | GP7
-------- | --- | --- | --- | --- | --- | --- | --- | ---
keypress | '1' | '2' | '3' | '4' | '5' | '6' | '7' | '8'
hex code | 0x01 | 0x02 | 0x04 | 0x08 | 0x10 | 0x20 | 0x40 | 0x80
joystick button | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 

## The Input GPIOs
The input pins are: GP0-GP7. The pull-up function is active on all the inputs to allow direct interfacing to pushbutton switches.

## Using NO/NC Button Contacts
By default, the *ezResponseBox* operates with Normally Open (NO) contacts. If at least one connected switch is of the Normally Closed (NC) type, the *ezResponseBox* will detect this upon startup (immediately after connecting to the USB port), resulting in the inversion of all logic input readings. To maintain simplicity, avoid mixing NO and NC contacts. When using NC-type contacts, ensure that unused input pins are tied to the ground (GND pin).

## The Output GPIOs
The eight debounced inputs are mapped to eight digital outputs, specifically GP8 to GP15. The logic state of these outputs can be inverted (refer to *Configuration Settings* below). The logic level is 3.3V; therefore, level converters and/or line drivers are required to interface with external 5V TTL logic or LED indicators.

## Configuration Settings
Upon connecting the *ezResponseBox* to a computer’s USB port, multiple devices may register with the operating system. The only active device is the one selected via jumper wires or DIP switches. Configuration is established at power-up. Refer to the function table below for detailed configuration settings.

![ezResponseBox_bb.png](ezResponseBox_bb.png "wiring diagram")

GPIO-pin  | open input | input tied to GND with jumper wire or DIP-switch
--------- | ---------- | ------------------------------------------------
GPIO18 | select keyboard device | select joystick device
GPIO19 | select numerical keys (mode-I) | select hexadecimal digits (mode-II)
GPIO20 | debouncing=ON | debouncing=OFF
GPIO21 | positive logic outputs | negative logic outputs

## Preparing your Raspberry Pico
Hookup one or more buttons to your Pico. Connect the Pico to the PC while pressing and holding the BOOTSEL button. A mass storage device will pop up. Drag the uf2 firmware file into the drive and ready you are! The uf2 firmware file can be found under the release download on this Github page.
This firmware was tested with the Raspberry Pi Pico (without W).

## A 10$ Button Box
A two button response box | bottom side
------------------------- | -----------
![](ezResponseBox.png) | ![](ezResponseBox_bottomside.png)

## Known bugs
- Displays only one "0"-character in keyboard mode-II for the 0-state code in some text editors or terminal programs. In real, double zeros are sent.
- The color of the prototype:)

