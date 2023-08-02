#include "mpbAdaptor.h"

DbncdMPBttn::DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}, _dbncTimeOrigSett{dbncTimeOrigSett}
{
    char mpbttnPinChar[3]{};
    sprintf(mpbttnPinChar, "%0.2d", (int)_mpbttnPin);
    strcpy(_mpbPollTmrName, "PollMpbPin");
    strcat(_mpbPollTmrName, mpbttnPinChar);
    strcat(_mpbPollTmrName, "_tmr");

    if(_dbncTimeOrigSett < _stdMinDbncTime)
        _dbncTimeOrigSett = _stdMinDbncTime;
    _dbncTimeTempSett = _dbncTimeOrigSett;

    pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT);

}

unsigned long int DbncdMPBttn::getCurDbncTime(){

    return _dbncTimeTempSett;
}

bool DbncdMPBttn::getIsPressed(){
    //updIsPressed();   //With the FreeRTOS timer working there's no need for this explicit call
    return _isPressed;
}

bool DbncdMPBttn::getIsOn (){
    //updIsOn(); //With the FreeRTOS timer working there's no need for this explicit call

    return _isOn;
}

bool DbncdMPBttn::resetDbncTime(){

    return setDbncTime(_dbncTimeOrigSett);
}

bool DbncdMPBttn::setDbncTime(const unsigned long int &newDbncTime){
    bool result {false};

    if (newDbncTime >= _stdMinDbncTime){
        _dbncTimeTempSett = newDbncTime;
        result = true;
    }

    return result;
}

bool DbncdMPBttn::updIsPressed(){
    /*To be pressed the conditions are:
    1) For NO == true
        a)  _pulledUp == false
            digitalRead == HIGH
        b)  _pulledUp == true
            digitalRead == FALSE
    2) for NO == false
        a)  _pulledUp == false
            digitalRead == LOW
        b)  _pulledUp == true
            digitalRead == HIGH
    */
    bool result {false};
    bool tmpPinLvl {digitalRead(_mpbttnPin)};
    
    if (_typeNO == true){
        if (_pulledUp == false){
            if (tmpPinLvl == HIGH)
                result = true;
        }
        else{
            if (tmpPinLvl == LOW)
                result = true;
        }
    }
    else{
        //For NC MPBs
        if (_pulledUp == false){
            if (tmpPinLvl == LOW)
                result = true;
        }
        else{
            if (tmpPinLvl == HIGH)
                result = true;
        }
    }    
    _isPressed = result;

    return _isPressed;
}

bool DbncdMPBttn::updIsOn(){
    bool result {false};

    //updIsPressed();   //With the FreeRTOS timer working there's no need for this explicit call
    if(_isPressed){
        if(_wasPressed == false){
            //Started to be pressed
            _wasPressed = true;
            //_dbncTimerStrt = millis(); //Arduino standard, but not FreeRTOS standard
            _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= _dbncTimeTempSett){
            //if ((millis() - _dbncTimerStrt) >= _dbncTimeTempSett){    //Arduino standard, but not FreeRTOS standard
                result = true;
            }
        }
    }
    else{
        _wasPressed = false;
    }
    _isOn = result;

    return result;
}

bool DbncdMPBttn::begin(unsigned long int pollDelayMs) {
    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            mpbPollCallback);
        assert (mpbPollTmrHndl);
    }
    xTimerStart(mpbPollTmrHndl, portMAX_DELAY);

    return mpbPollTmrHndl != nullptr;
}

bool DbncdMPBttn::pause(){
    xTimerStop(mpbPollTmrHndl, portMAX_DELAY);

    return true;
}

bool DbncdMPBttn::resume(){
    xTimerReset( mpbPollTmrHndl, portMAX_DELAY);    //Equivalent to xTimerStart()

    return true;
}

bool DbncdMPBttn::end(){
    xTimerStop(mpbPollTmrHndl, portMAX_DELAY);
    xTimerDelete(mpbPollTmrHndl, portMAX_DELAY);
    mpbPollTmrHndl = nullptr;

    return true;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    DbncdMPBttn *obj = (DbncdMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    obj->updIsPressed();
    obj->updIsOn();

    return;
}

unsigned long int DbncdDlydMPBttn::getStrtDelay(){
    
    return _strtDelay;
}

bool DbncdDlydMPBttn::setStrtDelay(unsigned long int newStrtDelay){
    _strtDelay = newStrtDelay;

    return true;
}

bool DbncdDlydMPBttn::updIsOn(){
    bool result {false};

    updIsPressed();

    if(_isPressed){
        if(_wasPressed == false){
            //Started to be pressed
            _wasPressed = true;
            _dbncTimerStrt = millis();
        }
        else{
            if ((millis() - _dbncTimerStrt) >= (_dbncTimeTempSett + _strtDelay)){
                result = true;
            }
        }
    }
    else{
        _wasPressed = false;
    }
    _isOn = result;

    return result;
}


SnglSrvcMPBttn::SnglSrvcMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTime)
{
}

bool SnglSrvcMPBttn::getSrvcPend(){
    
    return _servicePend;
}

bool SnglSrvcMPBttn::notifySrvd(){
    bool result {false};

    if (_servicePend){
        _servicePend = false;
        _wasPressed = true;
        result = true;
    }

    return result;
}

bool SnglSrvcMPBttn::updIsOn(){
    bool result {false};
    
    if (!_servicePend){
        if (DbncdMPBttn::updIsOn()){
            if (_released == true){
                _servicePend = true;
                _released = false;
                result = true;
            }
        }
        else{
            _released = true;
        }
    }
    else{
        result = true;
    }

    return result;
}


/*
AutoRptCntlMPBttn::AutoRptCntlMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTime, unsigned long int rptRate, bool autoRptOn)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTime), _autoRptOn {autoRptOn}
{
    if (rptRate <_minRptRate)
        _rptRate = _minRptRate;
    else
        _rptRate = rptRate;    
}

bool AutoRptCntlMPBttn::setAutoRptRate(unsigned long int newRptRate){
    bool result {false};

    if (newRptRate >= _minRptRate){
        _rptRate = newRptRate;
        result = true;
    }

    return result;
}
bool AutoRptCntlMPBttn::notifySrvd(){
    bool result {false};

    if (_servicePend){
        _servicePend = false;
        if(_autoRptOn){
            _rearmed = true;
            _dbncTimerStrt = millis();
            _wasPressed = true;
        }
        result = true;
    }

    return result;
}

bool AutoRptCntlMPBttn::updValidPress()
{
    bool result {false};
    
    updIsPressed();

    int pinState {digitalRead(_mpbttnPin)};
    bool nowPushed {_isPressed};    //Button is pushed == true
    if (!_servicePend){
        //There's no Service pending of being treated
        if (nowPushed){
            if (_autoRptOn || (!_autoRptOn && _rearmed) ){
                if(_wasPressed){
                    //It was already being pushed, timer is already running
                    if ((millis() - _dbncTimerStrt) >= _dbncTimeOrigSett){
                        _servicePend = true;
                        _rearmed = false;
                    }
                }
                else{
                    //It wasn't already pushed, timer must be started and status changed
                    _dbncTimerStrt = millis();
                    _wasPressed = true;
                }
            }
        }
        else{
            _wasPressed = false;
            if (!_autoRptOn)
                _rearmed = true;
        }
        result = true;
    }
    else{
        //Could not be treated as a Service is pending of treatment        
    }

    return result;

}

*/