#include "mpbAdaptor.h"

DbncdMPBttn::DbncdMPBttn(uint8_t mpbttnPin, bool pulledUp, bool typeNO, unsigned long int dbncTimeOrigSett)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}
{
    if(dbncTimeOrigSett < _stdMinDbncTime)
        _dbncTimeOrigSett = _stdMinDbncTime;
    else
        _dbncTimeOrigSett = dbncTimeOrigSett;
    _dbncTimeTempSett = _dbncTimeOrigSett;

    pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT);
}

unsigned long int DbncdMPBttn::getCurDbncTime(){

    return _dbncTimeTempSett;
}

bool DbncdMPBttn::getIsHit(){
    updIsHit();
    return _isHit;
}

bool DbncdMPBttn::getIsPressed (){
    updIsPressed();

    return _isPressed;
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

void DbncdMPBttn::updIsHit(){
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
    _isHit = result;

    return;
}

bool DbncdMPBttn::updIsPressed(){
    bool result {false};

    updIsHit();

    if(_isHit){
        if(_wasHit == false){
            //Started to be pressed
            _wasHit = true;
            _dbncTimerStrt = millis();
        }
        else{
            if ((millis() - _dbncTimerStrt) >= _dbncTimeTempSett){
                result = true;
            }
        }
    }
    else{
        _wasHit = false;
    }
    _isPressed = result;

    return result;
}

bool DbncdMPBttn::begin(TickType_t pollDelay)
{
    return false;
}

bool DbncdMPBttn::pause()
{
    return false;
}

bool DbncdMPBttn::resume()
{
    return false;
}

bool DbncdMPBttn::end()
{
    return false;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb)
{
}

unsigned long int DbncdDlydMPBttn::getStrtDelay(){
    return _strtDelay;
}

bool DbncdDlydMPBttn::setStrtDelay(unsigned long int newStrtDelay){
    _strtDelay = newStrtDelay;

    return true;
}

bool DbncdDlydMPBttn::updIsPressed(){
    bool result {false};

    updIsHit();

    if(_isHit){
        if(_wasHit == false){
            //Started to be pressed
            _wasHit = true;
            _dbncTimerStrt = millis();
        }
        else{
            if ((millis() - _dbncTimerStrt) >= (_dbncTimeTempSett + _strtDelay)){
                result = true;
            }
        }
    }
    else{
        _wasHit = false;
    }
    _isPressed = result;

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
        _wasHit = true;
        result = true;
    }

    return result;
}

bool SnglSrvcMPBttn::updIsPressed(){
    bool result {false};
    
    if (!_servicePend){
        if (DbncdMPBttn::updIsPressed()){
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
            _wasHit = true;
        }
        result = true;
    }

    return result;
}

bool AutoRptCntlMPBttn::updValidPress()
{
    bool result {false};
    
    updIsHit();

    int pinState {digitalRead(_mpbttnPin)};
    bool nowPushed {_isHit};    //Button is pushed == true
    if (!_servicePend){
        //There's no Service pending of being treated
        if (nowPushed){
            if (_autoRptOn || (!_autoRptOn && _rearmed) ){
                if(_wasHit){
                    //It was already being pushed, timer is already running
                    if ((millis() - _dbncTimerStrt) >= _dbncTimeOrigSett){
                        _servicePend = true;
                        _rearmed = false;
                    }
                }
                else{
                    //It wasn't already pushed, timer must be started and status changed
                    _dbncTimerStrt = millis();
                    _wasHit = true;
                }
            }
        }
        else{
            _wasHit = false;
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