# **Momentary Push Buttons to Switches** Library (mpbToSwitch)
## An Arduino-ESP32 library that builds physical electric switches replacements out of simple push buttons.  
By using just a push button (a.k.a. momentary switches or momentary buttons) the classes implemented in the library will manage the sensing of different parameters to **simulate the behavior of standard electromechanical switches**. Those parameters include presses, timings, counters or secondary inputs as needed.

The push button state is updated independently by a standard FreeRTOS timer (or ESP-RTOS in this case), that keeps the ON/OFF state of the objects created constantly updated. The timer setup is managed by in-class methods, including the possibility to pause, resume or end the timer of each object independently of the others.  
Each class offers a wide range of methods to set, read and modify every significant aspect of each switch mechanism type simulated, and the signal received from the push button is debounced for a correct behavior of the event processed.  

The benefits of the use of those simulated switches/objects are not just economic, as push buttons come in a wide range of prices and qualities. In a lighting automation project, for example, detecting through the daily use that an installed switch was not the best choice when a physical switch is involved, requires for the correction to a best suited kind of switch a bunch of activities to get the change of the switch unit done: buying a different best suited switch, uninstalling the first, maybe modifying the cabling, installing the second switch... in this case it's just about changing the class of the object, or modifying the instantiation parameters if needed, and the switch is already changed!! Want to keep the timed light on longer? Just a parameter. Want to turn off the pilot light because it bothers? Another parameter. Want to be sure the door mechanism isn't kept unlocked with an adhesive tape? Change the switch class.  
Just to add possibilities, consider that everywhere the **"momentary push button"** is mentioned, any kind of momentary activation signal provider might be used instead: touch sensors, PIR sensors, and so on...  

### The library implements -at this moment- the following switches mechanisms: ###  
* **Debounced Momentary Button**  (a.k.a. Momentary switch, a.k.a. Pushbutton), keeps the ON state since the moment the signal is stable (debouncing process) and until the moment the switch is released. It's implemented in the **DbncdMPBttn** class.
* **Debounced Delayed Momentary Button**, keeps the ON state since the moment the signal is stable (debouncing process), plus a delay added, and until the moment the push button is released. The reasons to add the delay are design related and are usually used to avoid unintentional presses, or to give some equipment (load) that needs time between repeated activations the benefit of the pause. If the push button is released before the delay configured no press is registered at all. The delay time in this class as in the other that implement it, can be zero (0). It's implemented in the **DbncdDlydMPBttn** class.  
* **Time Voidable Momentary Button**, keeps the ON state since the moment the signal is stable (debouncing process), plus a delay added, and until the moment the push button is released, or until a preset time in the ON state is reached. Then the switch will return to the Off position until the push button is released and pushed back. This kind of switches are used to activate limited resources related management or physical safety devices, and the possibility of a physical blocking of the switch to extend the ON signal artificially is highly undesired. Water valves, door unlocking mechanisms, hands-off security mechanisms, high power heating devices are some of the usual uses for these type of switches. It's implemented in the **TmVdblMPBttn** class.  
* **Toggle switch** (a.k.a. alternate, a.k.a. latched), keeps the ON state since the moment the signal is stable (debouncing process), and keeps the ON state after the push button is released and until it is pressed once again. So this simulates a simple On-Off switch like the ones used to turn on/off a room light. One of the best things about the software simulated switch is that any amount of switches can be set up in a parallel configuration, so that an unlimited number of entrances or easy accessible points can each have a switch to turn on/off the same resource. It's implemented in the **LtchMPBttn** class.  
* **Timer toggled** (a.k.a. timer switch), keeps the ON state since the moment the signal is debounced, and keeps the state during a set time, the switch time is set at instantiation, and can be modified through the provided methods. The switch implementation gives the option to allow to reset the timer before it gets to the end if the push button is pressed. It's implemented in the **TmLtchMPBttn** class.  
* **Hinted Timer toggled** (a.k.a. staircase timer switch), keeps the ON state since the moment the signal is debounced, and keeps the state during a set time, the switch time is set at instantiation, and can be modified through the provided methods. The switch implementation gives the option to allow to reset the timer before it gets to the end if the push button is pressed, the option to give a warning when the time is close to the end through a second output (remaining time is defined as a percentage of the total ON time and it's configurable), and the possibility to set a third signal on while the switch is off, just like the pilot light (hint) in a staircase timer switch. The warning signal is independent of the off hint. It's implemented in the **HntdTmLtchMPBttn** class.  
* **External released toggle** (a.k.a. Emergency latched), keeps the ON state since the moment the signal is debounced, and until an external signal is received. This kind of switch is used when an "abnormal situation" demands the push of the switch on, but a higher authority is needed to reset it to Off from a different signal source. Fire, smoke, intrusion alarms and "last man locks" are some examples of the use of this switch. As the external release signal can be physical or logical generated it can be implemented to be received from a switch or a remote signal of any usual kind. It's implemented in the  **XtrnUnltchMPBttn** class.    
* ### More switches classes are already under development to be added. ###

The timer will periodically check the input pins associated to the objects and refresh the object's input situation. The classes provide a callback function to keep the behavior of the objects updated, a valid approach is to create a task for each object to repeatedly refresh the status of the outputs associated with each switch in an unattended fashion, or to create a single task to update a number of switches. Examples are provided for each of the classes, usually as pairs in each example to show possible interactions between different objects, the possible implementations with tasks or with a refreshing mechanism in the loop() (loopTask). The object input status checking can be paused, restarted and even ended. If the object's status checking is ended the timer entry will be deleted, to release resources.  
So this library implements the inner mechanisms of some switches, the hardware interface (the load connection, warning outputs, etc.) might be develop differently for every project, or the **_switchHIL_** library might be used, as it implements **Hardware Interface Layers** for some specific switches.


# **Included Methods for DbncdMPBttn class**

|Method | Parameters|
|---|---|
|**_DbncdMPBttn_** |None|
|**_DbncdMPBttn_** |uint8_t **mpbttnPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**)))|
|**begin()**|(unsigned long int **pollDelayMs**)|
|**end()**|None|
|**getCurDbncTime()**|None|
|**getIsOn()**|None|
|**init** |uint8_t **mpbttnPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**)))|
|**pause()**|None|
|**resetDbncTime()**|None|
|**resume()**|None|
|**setDbncTime()**|unsigned long int **newDbncTime**|
|**updIsOn()**|None|
|**updIsPressed()**|None|
|**updValidPressPend()**|None|  

---
### **DbncdMPBttn**(uint8_t **mpbttnPin**, bool **pulledUp**, bool **typeNO**, unsigned long int **dbncTimeOrigSett**)
### Description:  
Class constructor, create an instance of the class for each **Debounced Momentary Push Button**. There's no need to configure the input pin before calling the method, as the constructor takes care of the task.  
### Parameters:  
**mpbttnPin:** uint8_t (unsigned char), passes the pin number that is connected to the push button. The pin must be free to be used as a digital input.  
**pulledUp:** optional boolean, indicates if the input pin must be configured as INPUT_PULLUP (true, default value), or INPUT_PULLDOWN(false), to calculate correctly the expected voltage level in the input pin. The pin is configured by the constructor so no previous programming is needed. The pin must be free to be used as a digital input.  
**typeNO:** optional boolean, indicates if the mpb is a **Normally Open** switch (true, default value), or **Normally Closed** (false), to calculate correctly the expected level of voltage indicating the mpb is pressed.   
**dbncTimeOrigSett:** optional unsigned long integer (uLong), indicates the time (in milliseconds) to wait for a stable input signal before considering the mpb to be pressed. If no value is passed the constructor will assign the minimum value provided in the class, that is 50 milliseconds as it is a empirical value obtained in various published tests.  

### Return value:  
The object created.

### Use example:  
**`DbncdMPBttn myDDButton(21, true, true, 100);`**

---
---
### **begin**(unsigned long int pollDelayMs)
### Description:  
Attaches
### Parameters:  
**pollDelayMs:** unsigned long integer (ulong), passes the ue provided in the class, that is 50 milliseconds as it is a empirical value obtained in various published tests.  

### Return value:  
Eee

### Use example:  
**`myDDButton.begin(20);`**

---

---  
# **Added Methods for DbncdDlydMPBttn class**

|Method | Parameters|
|---|---|
|**_DbncdDlydMPBttn_** |None|
|**_DbncdDlydMPBttn_** |uint8_t **mpbttnPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**))))|
|**getStrtDelay()**|None|
|**init** |uint8_t **mpbttnPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**))))|
|**setStrtDelay()**|(unsigned long int **newStrtDelay**)|

---  
### **DbncdDlydMPBttn**(uint8_t **mpbttnPin**, bool **pulledUp**, bool **typeNO**, unsigned long int **dbncTimeOrigSett**, unsigned long int **strtDelay**)
### Description:  
Class constructor, create an instance of the class for each **Debounced Delayed Momentary Push Button**. There's no need to configure the input pin before calling the method, as the constructor takes care of the task.  
### Parameters:  
**mpbttnPin:** uint8_t (unsigned char), passes the pin number that is connected to the push button. The pin must be free to be used as a digital input.  
**pulledUp:** optional boolean, indicates if the input pin must be configured as INPUT_PULLUP (true, default value), or INPUT_PULLDOWN(false), to calculate correctly the expected voltage level in the input pin. The pin is configured by the constructor so no previous programming is needed. The pin must be free to be used as a digital input.  
**typeNO:** optional boolean, indicates if the mpb is a **Normally Open** switch (true, default value), or **Normally Closed** (false), to calculate correctly the expected level of voltage indicating the mpb is pressed.   
**dbncTimeOrigSett:** optional unsigned long integer (uLong), indicates the time (in milliseconds) to wait for a stable input signal before considering the mpb to be pressed. If no value is passed the constructor will assign the minimum value provided in the class, that is 50 milliseconds as it is a empirical value obtained in various published tests.  
**strtDelay:** optional unsigned long integer (uLong), indicates the time (in milliseconds) to wait after the debouncing is completed and before rising the ON flag. If no value is passed the constructor will assign 0 milliseconds, and the resulting behavior will be the same as implementing a DbncdMPBttn. If the mpb is released before completing the strtDelay time, no ON signal will be produced.  

### Return value:  
The object created.

### Use example:  
**`DbncdMPBttn myDDButton(21, true, true, 100, 500);`**

---
---  
# **Added Methods for LtchMPBttn class**

|Method | Parameters|
|---|---|
|**_LtchMPBttn_** |uint8_t **mpbttnPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**))))|
|**getUnlatchPend()**|None|
|**setUnlatchPend()**|None|
|**updUnlatchPend()**|None|

---  
---  
# **Added Methods for TmLtchMPBttn class**

|Method | Parameters|
|---|---|
|**_TmLtchMPBttn_** |uint8_t **mpbttnPin**, unsigned long int **actTime**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**))))|
|**getActTime()**|None|
|**setActTime()**|unsigned long int **newActTime**|
|**setTmrRestbl()**|bool **isRstbl**|

---  
---  
# **Added Methods for HntdTmLtchMPBttn class**

|Method | Parameters|
|---|---|
|**_HntdTmLtchMPBttn_** |uint8_t **mpbttnPin**, unsigned long int **actTime**(, unsigned int **wrnngPrctg**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**)))))|  
|**getPilotOn()**|None|
|**getWrnngOn()**|None|
|**setKeepPilot()**|bool **keepPilot**|
|**updPilotOn()**|None|
|**updWrnngOn()**|None|

---  
---  
# **Added Methods for XtrnUnLtchMPBttn class**

|Method | Parameters|
|---|---|
|**_XtrnUnLtchMPBttn_** |uint8_t **mpbttnPin**, uint8_t **unltchPin**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**(, bool **upulledUp**(, bool **utypeNO**(, unsigned long int **udbncTimeOrigSett**(, unsigned long int **ustrtDelay**))))))))|
|**_XtrnUnLtchMPBttn_** |uint8_t **mpbttnPin**, DbncDlydMPBttn **unltchBttn**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**))))|

---  
---  
# **Added Methods for TmVdblMPBttn class**

|Method | Parameters|
|---|---|
|**_TmVdblMPBttn_** |uint8_t **mpbttnPin**, unsigned long int **voidTime**(, bool **pulledUp**(, bool **typeNO**(, unsigned long int **dbncTimeOrigSett**(, unsigned long int **strtDelay**(, bool **isOnDisabled**)))))|
|**disable()**|None|
|**enable()**|None|
|**getIsEnabled()**|None|
|**getIsVoided()**|None|
|**getVoidTime()**|None|
|**setIsEnabled()**|bool **enabledValue**|
|**setIsVoided()**|bool **voidValue**|
|**setVoidTime()**|None|
|**updIsVoided()**|None|
