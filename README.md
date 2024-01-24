# ezResponseBox

*Open source response box for 1-8 buttons based on the Raspberry Pico*

Button reponse boxes are commonly used in Psychology experiments. Usually, response boxes are interfaced as a serial device or as Human Interface Device (HID) like a keyboard or joystick. The Raspberry Pico is a low cost micro-controller platform. Together with the desirable number of input buttons and a plastic box enclosure, a <50$ response box can be realized with similar specifications as for commercial equivalents.

The Easy Response Box or *ezResponseBox* can operate as both, a keyboard or a joystick type of response box with a typical latency of 1ms (=1000Hz update rate). The latency is 10 to 20 times lower than for a standard PC-keyboard.

The advantage of a joystick HID device is that the input status  of multiple switches is sent to the host in a single byte after a state change. The decoding of the buttons should be done in the application program.

In keyboard mode, the *ezResponseBox* sends a two character wide hexadecimal number ranging from `00` tot `FF`. Simultaneous key presses can be decoded in the application program.

The *ezResponseBox* scans eight digital inputs for reading momentary button knobs. Both, Normally Open (NO) and Normally Closed (NC) contacts can be used. The connected button contact type is scanned at power up. NC contacts are faster for detecting the onset of a response. The eight input channels are scanned at a rate of 10kHz and the readings are debounced with a FIR digital filter algorithm with a minimum of delay. The debounced information is resampled at the HID update rate. Only input state changes are sent to the computer.

The *ezResponseBox* can be interfaced for example with **PsychoPy** or **OpenSesame**.


## Specifications
- USB 2.0
- no drivers needed under Windows or Linux
- works as a keyboard or joystick HID composite device
- 1ms latency (minimum for HID)
- 10kHz input ports scan rate
- debouncing filter

GPIO pin vs. joystick button number:

GPIO-pin | GP0 | GP1 | GP2 | GP3 | GP4 | GP5 | GP6 | GP7
-------- | --- | --- | --- | --- | --- | --- | --- | ---
joystick button | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 

## Using NO/NC contacts
In the default situation, the *ezResponseBox* works with NO-contacts. If at least one connected switch is NC, the *ezResponseBox* will detect this at start up (immediately after connecting the USB port) and all logic input readings will be inverted. Please, don't mix up NO and NC contacts for the sake of simplicity. When using NC-contacts, please tie unused inputs to the ground (GND).


