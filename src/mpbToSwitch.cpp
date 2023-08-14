#include "mpbToSwitch.h"

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

    pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);

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
            digitalRead == LOW
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

    return _validPressPend;
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
    bool result {false};

    if (mpbPollTmrHndl){
        xTimerStop(mpbPollTmrHndl, portMAX_DELAY);
        result = true;
    }

    return result;
}

bool DbncdMPBttn::resume(){
    bool result {false};

    if (mpbPollTmrHndl){
        xTimerReset( mpbPollTmrHndl, portMAX_DELAY);    //Equivalent to xTimerStart()
        result = true;
    }

    return result;
}

bool DbncdMPBttn::end(){
    bool result {false};

    if (mpbPollTmrHndl){
        xTimerStop(mpbPollTmrHndl, portMAX_DELAY);
        xTimerDelete(mpbPollTmrHndl, portMAX_DELAY);
        mpbPollTmrHndl = nullptr;
        result = true;
    }

    return result;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    DbncdMPBttn *mpbObj = (DbncdMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updIsOn();

    return;
}

//=========================================================================> Class methods delimiter
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

    return DbncdMPBttn::updIsOn();
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
    DbncdDlydMPBttn *mpbObj = (DbncdDlydMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updIsOn();

    return;
}

//=========================================================================> Class methods delimiter
LtchMPBttn::LtchMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
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

bool LtchMPBttn::updUnlatchPend(){

    if(_validPressPend){
        if (_isOn){
            _unlatchPending = true;
            _validPressPend = false;
        }
    }

    return _unlatchPending;
}

bool LtchMPBttn::updIsOn(){

    if(_validPressPend){
        if (!_isOn){
            _isOn = !_isOn;
            _validPressPend = false;
        }
    }
    else if(_unlatchPending){
        if (_isOn){
            _isOn = !_isOn;
            _unlatchPending = false;
        }
    }

    return _isOn;
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

bool LtchMPBttn::setUnlatchPend(){

    if(!_unlatchPending){
        _unlatchPending = true;
        _validPressPend = false;
    }

    return _unlatchPending;
}

void LtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    LtchMPBttn *obj = (LtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    obj->updIsPressed();
    obj->updValidPressPend();
    obj->updUnlatchPend();
    obj->updIsOn();

    return;
}

//=========================================================================> Class methods delimiter
TmLtchMPBttn::TmLtchMPBttn(uint8_t mpbttnPin, unsigned long int actTime, unsigned int wrnngPrctg, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _srvcTime{actTime}, _wrnngPrctg{wrnngPrctg}
{
    if(_wrnngPrctg > 0)
        _wrnngMs = actTime * _wrnngPrctg/100;
    
}

bool TmLtchMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
}

bool TmLtchMPBttn::updValidPressPend(){

    return LtchMPBttn::updValidPressPend();
}

bool TmLtchMPBttn::updUnlatchPend(){
    if(_isOn){
        if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= _srvcTime){
            _unlatchPending = true;
            _validPressPend = false;
        }
    }

    return _unlatchPending;
}

bool TmLtchMPBttn::updIsOn() {

    if(_validPressPend){
        if (!_isOn){
            _srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
            _isOn = true;
        }
        else{
            if (_tmRstbl){
                _srvcTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
            }
        }
        _validPressPend = false;
    }
    else if(_unlatchPending){
        if (_isOn){
            _isOn = !_isOn;
            _unlatchPending = false;
        }
    }

    return _isOn;
}

bool TmLtchMPBttn::updWrnngOn(){

    if(_wrnngPrctg > 0){
        if (_isOn)
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= (_srvcTime - _wrnngMs))
                _wrnngOn = true;            
            else
                _wrnngOn = false;            
        else
            _wrnngOn = false;
    }
    
    return _wrnngOn;
}

void TmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    TmLtchMPBttn *mpbObj = (TmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updUnlatchPend();
    mpbObj->updIsOn();
    mpbObj->updWrnngOn();

    return;
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

bool TmLtchMPBttn::setWnngPinOut(uint8_t wrnngPinOut){
    _wnngPinOut = wrnngPinOut;
    pinMode(_wnngPinOut, OUTPUT);

    return true;
}

//=========================================================================> Class methods delimiter
XtrnUnltchMPBttn::XtrnUnltchMPBttn(uint8_t mpbttnPin, uint8_t unltchPin, 
        bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett, unsigned long int strtDelay,
        bool upulledUp, bool utypeNO, unsigned long int udbncTimeOrigSett, unsigned long int ustrtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _unLtchBttn(unltchPin, upulledUp, utypeNO, udbncTimeOrigSett, ustrtDelay)

{

}

bool XtrnUnltchMPBttn::begin(unsigned long int pollDelayMs)
{

    if (!mpbPollTmrHndl){        
        mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            XtrnUnltchMPBttn::mpbPollCallback);
        assert (mpbPollTmrHndl);
    }
    xTimerStart(mpbPollTmrHndl, portMAX_DELAY);
    
    _unLtchBttn.begin();

    return mpbPollTmrHndl != nullptr;
}

bool XtrnUnltchMPBttn::updUnlatchPend(){
    if(_isOn){
        if (_unLtchBttn.getIsOn()){
            _unlatchPending = true;
            _validPressPend = false;
        }
    }

    return _unlatchPending;
}

void XtrnUnltchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    XtrnUnltchMPBttn *mpbObj = (XtrnUnltchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updUnlatchPend();
    mpbObj->updIsOn();

    return;
}


//=========================================================================> Class methods delimiter

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

