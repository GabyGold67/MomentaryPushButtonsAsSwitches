# **PushButtonsLib**
An Arduino library to deal with Momentary Push Buttons (MPBs): debouncing, mandatory unpressing, time voiding and ignoring to simulate other common types of switches by software.

The switches to simulate are:
MPB (a.k.a. Momentary switch, a.k.a. Pushbutton),
Toggle (a.k.a. alternate, a.k.a. latched),
Timed toggle (with/without warning, with/without external hint)
External released toggle (a.k.a. Emergency latched),
Inverter (single or multipole)
3-ways (a.k.a. staircase),
multiPole (independent or an option to other switches?)

The master idea is to poll a MPB object instead of a physical pin, even using the physical pinNumber transparently. Polling the object will return a clean signal 
from a MPB debounced, delay corrected, simulating button press and button release for the MPB to act as different kind of physical switches.
The Off/On status of the switch will be updated and logically calculated by a timer triggered callback (FreeRTOS).
The switches will have the possibility to be updated from a begin() and until an end(), and suspend() and resume() polling