#include <mpbToSwitch.h>

DbncdMPBttn::DbncdMPBttn()
: _mpbttnPin{0}, _pulledUp{true}, _typeNO{true}, _dbncTimeOrigSett{0}
{
}

DbncdMPBttn::DbncdMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}, _dbncTimeOrigSett{dbncTimeOrigSett}
{
    char mpbttnPinChar[3]{};
    sprintf(mpbttnPinChar, "%0.2d", (int)_mpbttnPin);
    strcpy(_mpbPollTmrName, "PollMpbPin");
    strcat(_mpbPollTmrName, mpbttnPinChar);
    strcat(_mpbPollTmrName, "_tmr");

    if(_dbncTimeOrigSett < _stdMinDbncTime) //Best practice would impose failing the constructor (throwing an exeption or building a "zombie" object)
        _dbncTimeOrigSett = _stdMinDbncTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters
    _dbncTimeTempSett = _dbncTimeOrigSett;

    if(mpbttnPin > 0)
        pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
}

const unsigned long int DbncdMPBttn::getCurDbncTime() const{

    return _dbncTimeTempSett;
}

const bool DbncdMPBttn::getIsOn() const {

    return _isOn;
}

const bool DbncdMPBttn::getIsPressed() const {

    return _isPressed;
}

const bool DbncdMPBttn::getOutputsChange() const{

    return _outputsChange;
}

const TaskHandle_t DbncdMPBttn::getTaskToNotify() const{
    
    return _taskToNotifyHndl;
}

bool DbncdMPBttn::init(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett){
    char mpbttnPinChar[3]{};

    bool result {false};
    if (_mpbPollTmrName[0] == '\0'){
        _mpbttnPin = mpbttnPin;
        _pulledUp = pulledUp;
        _typeNO = typeNO;
        _dbncTimeOrigSett = dbncTimeOrigSett;

        sprintf(mpbttnPinChar, "%0.2d", (int)_mpbttnPin);
        strcpy(_mpbPollTmrName, "PollMpbPin");
        strcat(_mpbPollTmrName, mpbttnPinChar);
        strcat(_mpbPollTmrName, "_tmr");

        if(_dbncTimeOrigSett < _stdMinDbncTime)
            _dbncTimeOrigSett = _stdMinDbncTime;
        _dbncTimeTempSett = _dbncTimeOrigSett;

        if(mpbttnPin > 0)
            pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
        result = true;
    }
    
    return result;
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

bool DbncdMPBttn::setOutputsChange(bool newOutputChange){
    _outputsChange = newOutputChange;

    return _outputsChange;
}

bool DbncdMPBttn::updIsPressed(){
    /*To be 'pressed' the conditions are:
    1) For NO == true
        a)  _pulledUp == false ==> digitalRead == HIGH
        b)  _pulledUp == true ==> digitalRead == LOW
    2) For NO == false
        a)  _pulledUp == false ==> digitalRead == LOW
        b)  _pulledUp == true ==> digitalRead == HIGH
    */
    bool result {false};
    bool tmpPinLvl {digitalRead(_mpbttnPin)};
    
    if (_typeNO == true){
        //For NO MPBs
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
        if(_isOn == false){
            _isOn = true;
            _outputsChange = true;
        }
    }
    else{
        if(_isOn == true){
            _isOn = false;
            _outputsChange = true;
        }
    }

    return _isOn;
}

bool DbncdMPBttn::updValidPressPend(){
    bool result {false};

    if(_isPressed){
        if(_dbncTimerStrt == 0){    //It was not previously pressed
            //Started to be pressed
            _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= _dbncTimeTempSett){
                result = true;
            }
        }
    }
    else{
        if(_dbncTimerStrt > 0)
            _dbncTimerStrt = 0;
    }
    _validPressPend = result;

    return _validPressPend;
}

bool DbncdMPBttn::begin(const unsigned long int &pollDelayMs) {
    bool result {false};
    
    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback funtion to work
                DbncdMPBttn::mpbPollCallback);  //Callback function
            assert (_mpbPollTmrHndl);
        }
        xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);

        if (_mpbPollTmrHndl != nullptr)
            result = true;
    }

    return result;
}

bool DbncdMPBttn::pause(){
    bool result {false};

    if (_mpbPollTmrHndl){
        xTimerStop(_mpbPollTmrHndl, portMAX_DELAY);
        result = true;
    }

    return result;
}

bool DbncdMPBttn::resume(){
    bool result {false};

    if (_mpbPollTmrHndl){
        xTimerReset( _mpbPollTmrHndl, portMAX_DELAY);    //Equivalent to xTimerStart()
        result = true;
    }

    return result;
}

bool DbncdMPBttn::end(){
    bool result {false};

    if (_mpbPollTmrHndl){
        xTimerStop(_mpbPollTmrHndl, portMAX_DELAY);
        xTimerDelete(_mpbPollTmrHndl, portMAX_DELAY);
        _mpbPollTmrHndl = nullptr;
        result = true;
    }

    return result;
}

bool DbncdMPBttn::setTaskToNotify(TaskHandle_t newHandle){
    _taskToNotifyHndl = newHandle;

    return true;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    DbncdMPBttn *mpbObj = (DbncdMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updIsOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

//=========================================================================> Class methods delimiter
DbncdDlydMPBttn::DbncdDlydMPBttn()
:DbncdMPBttn()
{
}

DbncdDlydMPBttn::DbncdDlydMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:DbncdMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett), _strtDelay{strtDelay}
{
}

unsigned long int DbncdDlydMPBttn::getStrtDelay(){
    
    return _strtDelay;
}

bool DbncdDlydMPBttn::init(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay){
    bool result {false};

    result = DbncdMPBttn::init(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett);
    if (result)
        result = setStrtDelay(strtDelay);

    return result;
}

bool DbncdDlydMPBttn::setStrtDelay(const unsigned long int &newStrtDelay){
    _strtDelay = newStrtDelay;

    return true;
}

bool DbncdDlydMPBttn::updIsPressed(){
    
    return DbncdMPBttn::updIsPressed();
}

bool DbncdDlydMPBttn::updIsOn(){
    
    return DbncdMPBttn::updIsOn();
}

bool DbncdDlydMPBttn::updValidPressPend(){
    bool result {false};

    if(_isPressed){
        if(_dbncTimerStrt == 0){    //It was not previously pressed
            //Started to be pressed
            _dbncTimerStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _dbncTimerStrt) >= _dbncTimeTempSett + _strtDelay){
                result = true;
            }
        }
    }
    else{
        if(_dbncTimerStrt > 0)
            _dbncTimerStrt = 0;
    }
    _validPressPend = result;

    return result;
}

bool DbncdDlydMPBttn::begin(const unsigned long int &pollDelayMs){
    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback funtion to work
                DbncdDlydMPBttn::mpbPollCallback);
            assert (_mpbPollTmrHndl);
        }
        xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);

        return _mpbPollTmrHndl != nullptr;
    }

    return false;
}

void DbncdDlydMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    DbncdDlydMPBttn *mpbObj = (DbncdDlydMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updIsOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

//=========================================================================> Class methods delimiter
LtchMPBttn::LtchMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{
}

bool LtchMPBttn::begin(const unsigned long int &pollDelayMs){
    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback function to work
                LtchMPBttn::mpbPollCallback);
            assert (_mpbPollTmrHndl);
        }
        xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);

        return _mpbPollTmrHndl != nullptr;
    }

    return false;
}

const bool LtchMPBttn::getUnlatchPend() const{

    return _unlatchPending;
}

bool LtchMPBttn::setUnlatchPend(){
    if(!_unlatchPending){
        _unlatchPending = true;
        _validPressPend = false;
    }

    return _unlatchPending;
}

bool LtchMPBttn::updIsOn(){
    if(_validPressPend){
        if (!_isOn){
            _isOn = true;
            _validPressPend = false;
            _outputsChange = true;
        }
    }
    else if(_unlatchPending){
        if (_isOn){
            _isOn = false;
            _unlatchPending = false;
            _outputsChange = true;
        }
    }

    return _isOn;
}

bool LtchMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
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

bool LtchMPBttn::updValidPressPend(){
    if(_isPressed){
        if(!_releasePending){
            if(_dbncTimerStrt == 0){    //It was not previously pressed
                //Started to be pressed
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
        if(_dbncTimerStrt > 0)
            _dbncTimerStrt = 0;
        if(_releasePending)
            _releasePending = false;
    }

   return _validPressPend;
}

void LtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){    
    LtchMPBttn *mpbObj = (LtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updUnlatchPend();
    mpbObj->updIsOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

//=========================================================================> Class methods delimiter
TmLtchMPBttn::TmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _srvcTime{actTime}
{
    if(_srvcTime < 100) //Best practice would impose failing the constructor (throwing an exeption or building a "zombie" object)
        _srvcTime = 100;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters

}

const unsigned long int TmLtchMPBttn::getActTime() const{

    return _srvcTime;
}

bool TmLtchMPBttn::setActTime(const unsigned long int &newActTime){
    bool result {false};

    if (newActTime > 100){  //The minimum activation time is 100 millisecs
        _srvcTime = newActTime;
        result = true;
    }

    return result;
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
            _outputsChange = true;
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
            _isOn = false;
            _unlatchPending = false;
            _outputsChange = true;
        }
    }

    return _isOn;
}

void TmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    TmLtchMPBttn *mpbObj = (TmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updUnlatchPend();
    mpbObj->updIsOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

bool TmLtchMPBttn::begin(const unsigned long int &pollDelayMs){
    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work
            TmLtchMPBttn::mpbPollCallback);
        assert (_mpbPollTmrHndl);
    }
    xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);

    return _mpbPollTmrHndl != nullptr;
}

bool TmLtchMPBttn::setTmerRstbl(const bool &isRstbl){
    _tmRstbl = isRstbl;

    return true;
}

//=========================================================================> Class methods delimiter
HntdTmLtchMPBttn::HntdTmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const unsigned int &wrnngPrctg, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:TmLtchMPBttn(mpbttnPin, actTime, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _wrnngPrctg{wrnngPrctg}
{
        _wrnngMs = (_srvcTime * _wrnngPrctg) / 100;   
}

bool HntdTmLtchMPBttn::updPilotOn(){
    if (_keepPilot){
        if(!_isOn && !_pilotOn){
                _pilotOn = true;
                _outputsChange = true;
            }
        else if(_isOn && _pilotOn){
            _pilotOn = false;
            _outputsChange = true;
        }
    }
    else{
        if(_pilotOn){
            _pilotOn = false;
            _outputsChange = true;
        }
    }

    return _pilotOn;
}

bool HntdTmLtchMPBttn::updWrnngOn(){
    if(_wrnngPrctg > 0){
        if (_isOn)
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _srvcTimerStrt) >= (_srvcTime - _wrnngMs))
                if(_wrnngOn == false){
                    _wrnngOn = true;            
                    _outputsChange = true;
                }
            else{
                if(_wrnngOn == true){
                    _wrnngOn = false;            
                    _outputsChange = true;                    
                }
            }
        else
            if(_wrnngOn == true){
                _wrnngOn = false;
                _outputsChange = true;                    
                }
    }
    
    return _wrnngOn;
}

const bool HntdTmLtchMPBttn::getWrnngOn() const{
    
    return _wrnngOn;
}

const bool HntdTmLtchMPBttn::getPilotOn() const{

    return _pilotOn;
}

bool HntdTmLtchMPBttn::setActTime(const unsigned long int &newActTime){
    bool result {true};

    if (newActTime != _srvcTime){
        result = TmLtchMPBttn::setActTime(newActTime);
        if (result)
            _wrnngMs = (_srvcTime * _wrnngPrctg) / 100;  //If the _srvcTime was changed, the _wrnngMs must be updated as it's a percentage of the first 
    }

    return result;
}

bool HntdTmLtchMPBttn::setKeepPilot(const bool &newKeepPilot){
    _keepPilot = newKeepPilot;
    
    return _keepPilot;
}

bool HntdTmLtchMPBttn::begin(const unsigned long int &pollDelayMs){
    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            HntdTmLtchMPBttn::mpbPollCallback);
        assert (_mpbPollTmrHndl);
    }
    xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);

    return _mpbPollTmrHndl != nullptr;
}

bool HntdTmLtchMPBttn::updIsPressed(){

    return DbncdMPBttn::updIsPressed();
}

bool HntdTmLtchMPBttn::updUnlatchPend(){

    return TmLtchMPBttn::updUnlatchPend();
}

bool HntdTmLtchMPBttn::updIsOn() {
    
    return TmLtchMPBttn::updIsOn() ;
}

bool HntdTmLtchMPBttn::updValidPressPend(){

    return LtchMPBttn::updValidPressPend();
}

void HntdTmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    HntdTmLtchMPBttn *mpbObj = (HntdTmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updUnlatchPend();
    mpbObj->updIsOn();
    mpbObj->updWrnngOn();
    mpbObj->updPilotOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

//=========================================================================> Class methods delimiter
XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin, const uint8_t &unltchPin, 
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay,
        const bool &upulledUp, const bool &utypeNO, const unsigned long int &udbncTimeOrigSett, const unsigned long int &ustrtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _unLtchBttn(unltchPin, upulledUp, utypeNO, udbncTimeOrigSett, ustrtDelay)
{

}

XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin, const DbncdDlydMPBttn &unltchBttn, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{

}

bool XtrnUnltchMPBttn::begin(const unsigned long int &pollDelayMs){
    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            XtrnUnltchMPBttn::mpbPollCallback);
        assert (_mpbPollTmrHndl);
    }
    xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
    
    _unLtchBttn.begin();

    return _mpbPollTmrHndl != nullptr;
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
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

//=========================================================================> Class methods delimiter
VdblMPBttn::VdblMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay, const bool &isOnDisabled)
:DbncdDlydMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _isOnDisabled{isOnDisabled}
{
}

VdblMPBttn::~VdblMPBttn(){
}

const bool VdblMPBttn::getIsEnabled() const{

    return _isEnabled;
}

const bool VdblMPBttn::getIsVoided() const{

    return _isVoided;
}

bool VdblMPBttn::setIsVoided(const bool &newVoidValue){
    _isVoided = newVoidValue;

    return _isVoided;
}

bool VdblMPBttn::setIsEnabled(const bool &newEnabledValue){
    if (!newEnabledValue){
        if(_isEnabled){
            if(_isOnDisabled){
                if(_isOn == false){
                    _isOn = true;
                    _outputsChange = true;
                }
            }
            else{
                if (_isOn == true){
                    _isOn = false;
                    _outputsChange = true;
                }
            }
            pause();
        }            
    }
    else{
        resume();
    }
    if(_isEnabled != newEnabledValue)
        _outputsChange = true;
    _isEnabled = newEnabledValue;

    return _isEnabled;
}

bool VdblMPBttn::enable(){

    return setIsEnabled(true);
}

bool VdblMPBttn::disable(){

    return setIsEnabled(false);
}

//=========================================================================> Class methods delimiter
TmVdblMPBttn::TmVdblMPBttn(const uint8_t &mpbttnPin, unsigned long int voidTime, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay, const bool &isOnDisabled)
:VdblMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay, isOnDisabled), _voidTime{voidTime}
{
}

TmVdblMPBttn::~TmVdblMPBttn(){
}

void TmVdblMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCb){
    TmVdblMPBttn *mpbObj = (TmVdblMPBttn*)pvTimerGetTimerID(mpbTmrCb);
    mpbObj->updIsPressed();
    mpbObj->updValidPressPend();
    mpbObj->updIsVoided();
    mpbObj->updIsOn();
    if (mpbObj->getOutputsChange()){
        if(mpbObj->getTaskToNotify() != nullptr)
            xTaskNotifyGive(mpbObj->getTaskToNotify());
        mpbObj->setOutputsChange(false);
    }

    return;
}

const unsigned long int TmVdblMPBttn::getVoidTime() const{

    return _voidTime;
}

bool TmVdblMPBttn::setVoidTime(const unsigned long int &newVoidTime){
    bool result{false};

    if(newVoidTime > 0){
        _voidTime = newVoidTime;
        result = true;
    }
    
    return result;
}

bool TmVdblMPBttn::begin(const unsigned long int &pollDelayMs){
    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback funtion to work
            TmVdblMPBttn::mpbPollCallback);
        assert (_mpbPollTmrHndl);
    }
    xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
    
    return _mpbPollTmrHndl != nullptr;
}

bool TmVdblMPBttn::updIsPressed(){
    
    return DbncdDlydMPBttn::updIsPressed();
}

bool TmVdblMPBttn::setIsVoided(const bool &newVoidValue){
    if(newVoidValue){
        _voidTmrStrt = (xTaskGetTickCount() / portTICK_RATE_MS) -(_voidTime + 1);
    }
    if(_isVoided != newVoidValue)
        _outputsChange = true;
    _isVoided = newVoidValue;

    return _isVoided;
}

bool TmVdblMPBttn::updValidPressPend(){
    
    return DbncdDlydMPBttn::updValidPressPend();
}

bool TmVdblMPBttn::updIsOn() {
    if (!_isVoided){
        if (_validPressPend){
            if(_isOn == false){
                _isOn = true;
                _outputsChange = true;                        
            }
        }
        else{
            if(_isOn == true){
                _isOn = false;
                _outputsChange = true;
            }
        }
    }
    else{
        if (_isOn == true){
            _isOn = false;
            _outputsChange = true;
        }
    }

    return _isOn;
}

bool TmVdblMPBttn::updIsVoided(){
    //if it's pressed
        //if the pressing timer is running
            // if the pressing timer is greater than the debounceTime + strtDelay + voidTime
                //Set isVoided to true
                //Set isOn to false (the updateIsOn() will have to check _isVoided to prevent reverting back on)
    bool result {false};

    if(_isPressed){
        if(_voidTmrStrt == 0){    //It was not previously pressed
            //Started to be pressed
            _voidTmrStrt = xTaskGetTickCount() / portTICK_RATE_MS;
        }
        else{
            if (((xTaskGetTickCount() / portTICK_RATE_MS) - _voidTmrStrt) >= (_voidTime)){ // + _dbncTimeTempSett + _strtDelay
                result = true;
            }
        }
    }
    else{
        _voidTmrStrt = 0;
    }
    if(_isVoided != result)
        _outputsChange = true;

    _isVoided = result;

    return _isVoided;
}
