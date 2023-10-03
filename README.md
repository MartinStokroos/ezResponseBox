# ezResponseBox

*Open source response box for 1-8 buttons based on a Raspberry Pico*

Reponse Boxes are commonly used in Psychology experiments. Often response boxes are interfaced serial or as a human interface device like a keyboard or joystick.
The Easy Response Box or *ezResponseBox* is a joystick type response box with a 1ms latency (1000Hz). The latency is 10 to 20 times lower than for a normal PC keyboard. The advantage of a joystick HID device is that a status change of multiple switches is sent to the host in one data package. The switch decoding should be done in the application program.

The ezResponseBox scans 8 digital inputs for reading momentary button knobs. Both, Normally Open (NO) and Normally Closed (NC) contacts can be used. The connected button switch type is scanned at power up. NC buttons are faster to detect the onset of a response. The eight input channels are scanned at 10kHz and the readings are debounced with a smart digital filter algorithm.

The ezResponseBox could be interfaced with ease for example with **PsychoPy** or **OpenSesame**.

The Raspberry Pico is a low cost micro-controller platform. Together with a selectable amount of button switches and a plastic box enclosure, a <50$ response box could be realized with similar specifications as commercial equivalents.

## Specifications
- USB 2.0
- no drivers needed
- keyboard HID composite device
- 1ms latency (lowest for the HID standard)
- 10kHz input scan rate
- smart debouncing filter
- GPIO pin vs. joystick button number:

GPIO-pin | GP0 | GP1 | GP2 | GP3 | GP4 | GP5 | GP6 | GP7
--- | --- | --- | --- | --- | --- | --- | --- | ---
button | 1   | 2   | 3   | 4   | 5   | 6   | 7   | 8

