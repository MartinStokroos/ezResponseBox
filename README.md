# ezResponseBox

*Open source response box for 1-8 buttons based on a Raspberry Pico*

Button reponse boxes are frequently used in Psychology experiments. Often response boxes are interfaced as a serial device or as Human Interface Device (HID), like a keyboard or joystick. The Raspberry Pico is a low cost micro-controller platform. Together with the desirable number of button switches and a plastic box enclosure, a <50$ response box could be realized with similar specifications as commercial equivalents.

The Easy Response Box or *ezResponseBox* can operate both, as a keyboard as well as a joystick type of response box with a 1ms typical latency time (=1000Hz update rate). The latency is 10 to 20 times lower than for a standard PC-keyboard. The advantage of a joystick HID device is that the status change of multiple switches is sent to the host in a single byte and the button decoding should be done in the application program.

The ezResponseBox scans eight digital inputs for reading momentary button knobs. Both, Normally Open (NO) and Normally Closed (NC) contacts can be used. The connected button contact type is scanned at power up. NC contacts are faster for detecting the onset of a response. The eight input channels are scanned at a rate of 10kHz and the readings are debounced with a FIR digital filter algorithm with minimum delay. The debounced information is resampled at the HID update rate. Only the state changes of the inputs are sent to the computer.

The *ezResponseBox* can be interfaced for example with **PsychoPy** or **OpenSesame**.


## Specifications
- USB 2.0
- no drivers needed
- keyboard or joystick HID composite device
- 1ms latency (minimum for HID)
- 10kHz input scan rate
- debouncing filter
- GPIO pin vs. joystick button number:

GPIO-pin | GP0 | GP1 | GP2 | GP3 | GP4 | GP5 | GP6 | GP7
-------- | --- | --- | --- | --- | --- | --- | --- | ---
joystick button | 0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 
keyboard character | '1' | '2' | '3' | '4' | '5' | '6' | '7' 

