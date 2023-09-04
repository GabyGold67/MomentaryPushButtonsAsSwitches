#include <switchHIL.h>


//Declare function prototypes
static void udpSwitchStatus(void* argp);

//Declare an array of pointers to superclass objects, initialize with nullptr
// static HILSwitches *actSwtchsArr[MAX_SWITCHES] {};

HILSwitches::HILSwitches(){ //This default constructor of this superclass will be called first each time the constructor of the subclasses are invoked, 
                            //so it will check for the task that refreshes the outputs to be created

}

//=========================================================================> Class methods delimiter

StrcsTmrSwitch::StrcsTmrSwitch(HntdTmLtchMPBttn &lgcMPB, uint8_t loadPin, uint8_t wnngPin, uint8_t pltPin)
:_underlMPB{&lgcMPB}, _loadPin {loadPin}, _wrnngPin{wnngPin}, _pltPin{pltPin}
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

    //Add a pointer to the switch instantiated to the array of pointers of switches whose outputs must be updated
    if(switchesCount < MAX_SWITCHES){
        actSwtchsArr[switchesCount] = this;
        switchesCount++;
    }
            
    _underlMPB->begin(); //Set the logical underlying mpBttn to start updating it's inputs readings & output states
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

    if(_pltPin > 0){
        if(actvPilot != _actvPlt)
            _actvPlt = actvPilot;
    }
    else
        result = false;
    
    return result;
}

bool StrcsTmrSwitch::setActvWarning(bool actvWarning){
    bool result {true};

    if(_wrnngPin > 0){
        if(actvWarning != _actvWrnng)
            _actvPlt = actvWarning;
    }    
    else
        result = false;
    
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

//=========================================================================> Class methods delimiter

DbncdDlydSwitch::DbncdDlydSwitch(DbncdDlydMPBttn &lgcMPB, uint8_t loadPin)
:_underlMPB{&lgcMPB}, _loadPin{loadPin}
{
    //Set the output pins to the required states
    digitalWrite(_loadPin, LOW);   //Ensure the pin signal is down before setting as output for safety. Usually unneded as all pins are initiated openC, 
    pinMode(_loadPin, OUTPUT);

    //This is a repeated code from other HILSwitches subclasses, refactoring needed!?
    //Add a pointer to the switch instantiated to the array of pointers of switches whose outputs must be updated
    if(switchesCount < MAX_SWITCHES){
        actSwtchsArr[switchesCount] = this;
        switchesCount++;
    }
    _underlMPB->begin(); //Set the logical underlying mpBttn to start updating it's inputs readings & output states
}

bool DbncdDlydSwitch::updOutputs(){
    if(_underlMPB->getIsOn()){
        if(digitalRead(_loadPin) != HIGH)
            digitalWrite(_loadPin, HIGH);
    }
    else{
        if(digitalRead(_loadPin) != LOW)
            digitalWrite(_loadPin, LOW);
    }

    return true;
}

DbncdDlydMPBttn* DbncdDlydSwitch::getUnderlMPB(){
    
    return _underlMPB;
}


//==================================================================
static void udpSwitchStatus(void* argp){
  HILSwitches *mySwtch = (HILSwitches*)argp;  //Parameter casting: the parameter passed by the task is casted to its real type

  for (;;){
      mySwtch->updOutputs();
  }
}
//==================================================================
