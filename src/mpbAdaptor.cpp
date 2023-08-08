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

    return _isPressed;
}

bool DbncdMPBttn::getIsOn (){

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

    if (_validPressPend){
        _isOn = true;
    }
    else{
        _isOn = false;
    }

    return _isOn;
}

bool DbncdMPBttn::updValidPressPend(){
    bool result {false};

    if(_isPressed){
        if(_wasPressed == false){
            //Started to be pressed
            _wasPressed = true;
            _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= _dbncTimeTempSett){
                result = true;
            }
        }
    }
    else{
        _wasPressed = false;
    }
    _validPressPend = result;

    return result;
}

bool DbncdMPBttn::begin(unsigned long int pollDelayMs) {
    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            DbncdMPBttn::mpbPollCallback);
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
    obj->updValidPressPend();
    obj->updIsOn();

    return;
}

DbncdDlydMPBttn::DbncdDlydMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett), _strtDelay{strtDelay}
{
}

unsigned long int DbncdDlydMPBttn::getStrtDelay(){
    
    return _strtDelay;
}

bool DbncdDlydMPBttn::setStrtDelay(unsigned long int newStrtDelay){
    _strtDelay = newStrtDelay;

    return true;
}

bool DbncdDlydMPBttn::updIsOn(){
    if (_validPressPend){
        _isOn = true;
    }
    else{
        _isOn = false;
    }

    return _isOn;
}

bool DbncdDlydMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
}

bool DbncdDlydMPBttn::updValidPressPend(){
    bool result {false};

    if(_isPressed){
        if(_wasPressed == false){
            //Started to be pressed
            _wasPressed = true;
            _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= _dbncTimeTempSett + _strtDelay){
                result = true;
            }
        }
    }
    else{
        _wasPressed = false;
    }
    _validPressPend = result;

    return result;
}

bool DbncdDlydMPBttn::begin(unsigned long int pollDelayMs){
    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            DbncdDlydMPBttn::mpbPollCallback);
        assert (mpbPollTmrHndl);
    }
    xTimerStart(mpbPollTmrHndl, portMAX_DELAY);

    return mpbPollTmrHndl != nullptr;
}

void DbncdDlydMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    DbncdDlydMPBttn *obj = (DbncdDlydMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    obj->updIsPressed();
    obj->updValidPressPend();
    obj->updIsOn();

    return;
}

LtchMPBttn::LtchMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

bool LtchMPBttn::updIsOn(){

    if(_validPressPend){
        _isOn = !_isOn;
        _validPressPend = false;
    }

    return _isOn;
}

bool LtchMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
}

bool LtchMPBttn::updValidPressPend(){

    if(_isPressed){
        if(!_releasePending){
            if(!_wasPressed){
                //Started to be pressed
                _wasPressed = true;
                _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
            }
            else{
                if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= (_dbncTimeTempSett + _strtDelay)){
                    _validPressPend = true;
                    _releasePending = true;
                }
            }
        }
    }
    else{
        _wasPressed = false;
        _releasePending = false;
    }

   return _validPressPend;
}

bool LtchMPBttn::begin(unsigned long int pollDelayMs){

    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            LtchMPBttn::mpbPollCallback);
        assert (mpbPollTmrHndl);
    }
    xTimerStart(mpbPollTmrHndl, portMAX_DELAY);

    return mpbPollTmrHndl != nullptr;
}

void LtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    LtchMPBttn *obj = (LtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    obj->updIsPressed();
    obj->updValidPressPend();
    obj->updIsOn();

    return;
}

TmLtchMPBttn::TmLtchMPBttn(uint8_t mpbttnPin, unsigned long int actTime, unsigned int wrnngPrctg, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _srvcTime{actTime}, _wrnngPrctg{wrnngPrctg}
{
    if(_wrnngPrctg > 0)
        _wrnngMs = actTime * _wrnngPrctg/100;
    
}

bool TmLtchMPBttn::begin(unsigned long int pollDelayMs){

    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            TmLtchMPBttn::mpbPollCallback);
        assert (mpbPollTmrHndl);
    }
    xTimerStart(mpbPollTmrHndl, portMAX_DELAY);

    return mpbPollTmrHndl != nullptr;
}

bool TmLtchMPBttn::getWrnngOn(){
    
    return _wrnngOn;
}

uint8_t TmLtchMPBttn::getWrnngPin(){
    return _wnngPinOut;
}

bool TmLtchMPBttn::setTmerRstbl(bool isRstbl){
    _tmRstbl = isRstbl;

    return true;
}

bool TmLtchMPBttn::updIsOn() {

    if(_validPressPend){
        if (!_isOn){
            _srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
            _wrnngOn = false;
            _isOn = true;
        }
        else{
            if (_tmRstbl){
                _srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
                _wrnngOn = false;

            }
        }
        _validPressPend = false;
    }
    if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= _srvcTime){
        _isOn = false;
    }

    return _isOn;
}

bool TmLtchMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
}

bool TmLtchMPBttn::updValidPressPend(){

    return LtchMPBttn::updValidPressPend();
}

bool TmLtchMPBttn::updWrnngOn(){

    if(_wrnngPrctg > 0){
        if (_isOn){
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= (_srvcTime - _wrnngMs)){
                _wrnngOn = true;
            }
        }
        else{
            _wrnngOn = false;
        }
    }
    
    return _wrnngOn;
}

bool TmLtchMPBttn::setWnngPinOut(uint8_t wrnngPinOut){
    _wnngPinOut = wrnngPinOut;
    pinMode(_wnngPinOut, OUTPUT);

    return true;
}

void TmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    TmLtchMPBttn *obj = (TmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    obj->updIsPressed();
    obj->updValidPressPend();
    obj->updIsOn();
    obj->updWrnngOn();

    return;
}




/*
SnglSrvcMPBttn::SnglSrvcMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
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

