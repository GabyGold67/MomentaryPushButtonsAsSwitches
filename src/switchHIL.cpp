#include <switchHIL.h>

//Declare an array of pointers to superclass objects, initialize with nullptr
static HILSwitches *actSwtchsArr[MAX_SWITCHES] {};

HILSwitches::HILSwitches(){ //This default constructor of this superclass will be called first each time the constructor of the subclasses are invoked, 
                            //so it will check for the task that refreshes the outputs to be created
    // if (!sHilTskHndl){  //The task has not been created yet, create it through a function that all switch classes must use

    //     //Set the task to keep the outputs updated and set the function name to the updater function
    //     rc = xTaskCreatePinnedToCore(
    //       updSwitchStatus,  //function to be called
    //       "UpdSwitchOutputs",  //Name of the task
    //       2048,   //Stack size (in bytes in ESP32, words in FreeRTOS), the minimum value is in the config file, for this is 768 bytes
    //       &myStairSwitch,  //Pointer to the parameters for the function to work with
    //       _exePrty,      //Priority level given to the task
    //       &sHilTskHndl, //Task handle
    //       xPortGetCoreID() //Run in the App Core if it's a dual core mcu (ESP32 only)
    //     );
    // assert(rc == pdPASS);
    // assert(sHilTskHndl);

    // }
}

//=========================================================================> Class methods delimiter

StrcsTmrSwitch::StrcsTmrSwitch(HntdTmLtchMPBttn &lgcMPB, uint8_t loadPin, uint8_t wnngPin, uint8_t pltPin)
:_underlMPB{&lgcMPB}, _loadPin{loadPin}, _wrnngPin{wnngPin}, _pltPin{pltPin}
{        
    //Set the output pins to the required states
    digitalWrite(_loadPin, LOW);   //Ensure the pin signal is down before setting as output for safety. Usually unneded as all pins are initiated openC, 
    pinMode(_loadPin, OUTPUT);

    if (_wrnngPin > 0){
        digitalWrite(_wrnngPin, LOW);   //Ensure the pin signal is down before setting as output for safety. Usually unneded as all pins are initiated openC, 
        pinMode(_wrnngPin, OUTPUT);
        _actvWrnng = true;
    }

    if (_pltPin > 0){
        digitalWrite(_pltPin, LOW);   //Ensure the pin signal is down before setting as output for safety. Usually unneded as all pins are initiated openC, 
        pinMode(_pltPin, OUTPUT);
        _actvPlt = true;
    }

    // if (!sHilTskHndl){  //The task has not been created yet, create it through a function that all switch classes must use
    //     createSwitchTask();
    // }

    //Add a pointer to the switch instantiated to the array of pointers of switches whose outputs must be updated
    for(int i {0}; i < MAX_SWITCHES; i++){
        if(actSwtchsArr[i] == nullptr){
            actSwtchsArr[i] = this;
            switchesCount++;
            break;
        }
    }
    _underlMPB->begin(); //Set the logical underlying mpBttn to start updating it's inputs/output states
    
}

bool StrcsTmrSwitch::updOutputs(){
    if(_underlMPB->getIsOn()){
        if(digitalRead(_loadPin) != HIGH)
            digitalWrite(_loadPin, HIGH);
    }
    else{
        if(digitalRead(_loadPin) != LOW)
            digitalWrite(_loadPin, LOW);
    }
    
    if(_wrnngPin > 0){
        if(_underlMPB->getWrnngOn()){
            if (_wrnngBlnks){   //If the warning output it's configured to blink
                if (_wrnngLstSwp == 0){ //If warning it's just been activated
                    _blnkOutputOn = true;   //Set (ensure) the warning output starts in ON condition
                    _wrnngLstSwp = xTaskGetTickCount() / portTICK_RATE_MS;  //Register blinking activity start on the stopwatch
                }
                else{   // not just activated
                    if (((xTaskGetTickCount() / portTICK_RATE_MS) - _wrnngLstSwp) >= _wrnngBlnkRate){   //Verify stopwatch time to change condition?
                        _blnkOutputOn = !_blnkOutputOn;     //Yes, swap _blnkOutputOn value
                        _wrnngLstSwp = xTaskGetTickCount() / portTICK_RATE_MS;  //Register time of last swap
                    }
                }
            }

            if((_wrnngBlnks == false) || (_wrnngBlnks && _blnkOutputOn)){
                if(digitalRead(_wrnngPin) != HIGH)
                    digitalWrite(_wrnngPin, HIGH);
                else if(digitalRead(_wrnngPin) != LOW)
                    digitalWrite(_wrnngPin, LOW);
            }
                        
        }
        else{
            if(digitalRead(_wrnngPin) != LOW)
                digitalWrite(_wrnngPin, LOW);
            if(_wrnngLstSwp > 0){   //Safe method to check if blinking was used, as the _wrnngBlnks flag might be changed while a blinking was taking place
                //Ensure blinking stopwatch reset for next use
                _wrnngLstSwp = 0;
                //Ensure the blinking output on is true for next use
                _blnkOutputOn = true;
            }
        }
    }

    if(_pltPin > 0){
        if(_underlMPB->getPilotOn()){
            if(digitalRead(_pltPin) != HIGH)
                digitalWrite(_pltPin, HIGH);
        }
        else{
            if(digitalRead(_pltPin) != LOW)
                digitalWrite(_pltPin, LOW);
        }
    }
    
    return true;
}

bool StrcsTmrSwitch::setActvPilot(bool actvPilot){
    bool result {true};

    if(actvPilot != _actvPlt){
        if(_pltPin > 0)
            _actvPlt = actvPilot;
        else
            result = false;
    }
    
    return result;
}

bool StrcsTmrSwitch::setActvWarning(bool actvWarning){
    bool result {true};

    if(actvWarning != _actvWrnng){
        if(_wrnngPin > 0)
            _actvPlt = actvWarning;
        else
            result = false;
    }
    
    return result;
}

const bool StrcsTmrSwitch::getActvPilot() const{

    return _actvPlt;
}

const bool StrcsTmrSwitch::getActvWarning() const{

    return _actvWrnng;
}

HntdTmLtchMPBttn* StrcsTmrSwitch::getUnderlMPB(){
    return _underlMPB;
}

bool StrcsTmrSwitch::setBlnkWrnng(bool newBlnkSett){
    if(newBlnkSett != _wrnngBlnks)
        _wrnngBlnks = newBlnkSett;

    return _wrnngBlnks;
}

bool StrcsTmrSwitch::setBlnkRate(unsigned long int newBlnkRate){
    bool result {false};
    
    if(newBlnkRate >= MIN_BLINK_RATE)
        _wrnngBlnkRate = newBlnkRate;
    else
        result = false;

    return result;
}

bool StrcsTmrSwitch::blinkWrnng(){
    
    return setBlnkWrnng(true);
}

bool StrcsTmrSwitch::noBlinkWrnng(){
    
    return setBlnkWrnng(false);
}

