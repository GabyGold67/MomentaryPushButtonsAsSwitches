#include <mpbToSwitch.h>

DbncdMPBttn::DbncdMPBttn()
: _mpbttnPin{0xFF}, _pulledUp{true}, _typeNO{true}, _dbncTimeOrigSett{0}
{
}

DbncdMPBttn::DbncdMPBttn(const uint8_t &mpbttnPin, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett)
: _mpbttnPin{mpbttnPin}, _pulledUp{pulledUp}, _typeNO{typeNO}, _dbncTimeOrigSett{dbncTimeOrigSett}
{

    if(mpbttnPin != 0xFF){
        char mpbttnPinChar[3]{};
        sprintf(mpbttnPinChar, "%0.2d", (int)_mpbttnPin);
        strcpy(_mpbPollTmrName, "PollMpbPin");
        strcat(_mpbPollTmrName, mpbttnPinChar);
        strcat(_mpbPollTmrName, "_tmr");

        if(_dbncTimeOrigSett < _stdMinDbncTime) //Best practice would impose failing the constructor (throwing an exeption or building a "zombie" object)
            _dbncTimeOrigSett = _stdMinDbncTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters
        _dbncTimeTempSett = _dbncTimeOrigSett;
        pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
    }
    else{
        _pulledUp = true;
        _typeNO = true;
        _dbncTimeOrigSett = 0;
    }
}

DbncdMPBttn::~DbncdMPBttn(){
    
    end();  // Stops the software timer associated to the object, deletes it's entry and nullyfies the handle to it befor destructing the object
}

void DbncdMPBttn::clrStatus(){
    /*To Resume operations after a pause() without risking generating false "Valid presses" and "On" situations,
    several attributes must be resetted to "Start" values*/
    _isPressed = false;
    _validPressPend = false;
    _dbncTimerStrt = 0;
    if (_isOn){
        _isOn = false;
        _outputsChange = true;
    }
    
    return;
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
    bool result {false};
    char mpbttnPinChar[3]{};

    if((_mpbttnPin == 0xFF) && (mpbttnPin != 0xFF)){
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

            pinMode(mpbttnPin, (pulledUp == true)?INPUT_PULLUP:INPUT_PULLDOWN);
            result = true;
        }
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

bool DbncdMPBttn::setOutputsChange(bool newOutputsChange){
    if(_outputsChange != newOutputsChange)
        _outputsChange = newOutputsChange;

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
    if(_validPressPend != result)
        _validPressPend = result;

    return result;
}

bool DbncdMPBttn::begin(const unsigned long int &pollDelayMs) {
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback funtion to work
                mpbPollCallback  //DbncdMPBttn::mpbPollCallback  //Callback function
            );
        }
        if (_mpbPollTmrHndl != nullptr){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

bool DbncdMPBttn::pause(){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (_mpbPollTmrHndl){
        tmrModResult = xTimerStop(_mpbPollTmrHndl, portMAX_DELAY);
        if(tmrModResult == pdPASS)
            result = true;
    }

    return result;
}

bool DbncdMPBttn::resume(){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (_mpbPollTmrHndl){
        tmrModResult = xTimerReset( _mpbPollTmrHndl, portMAX_DELAY);    //Equivalent to xTimerStart()
        if(tmrModResult == pdPASS)
            result = true;
    }

    return result;
}

bool DbncdMPBttn::end(){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (_mpbPollTmrHndl){
        tmrModResult = xTimerStop(_mpbPollTmrHndl, portMAX_DELAY);
        if(tmrModResult == pdPASS){
            tmrModResult = xTimerDelete(_mpbPollTmrHndl, portMAX_DELAY);
            if(tmrModResult == pdPASS){
                _mpbPollTmrHndl = nullptr;
                result = true;
            }
        }
    }

    return result;
}

bool DbncdMPBttn::setTaskToNotify(TaskHandle_t newHandle){
    bool result {true};

    if(_taskToNotifyHndl != newHandle)
        _taskToNotifyHndl = newHandle;
    if (newHandle == nullptr)
        result = false;

    return result;
}

void DbncdMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    DbncdMPBttn *mpbObj = (DbncdMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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
    if(_strtDelay != newStrtDelay)
        _strtDelay = newStrtDelay;

    return true;
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
    if(_validPressPend != result)
        _validPressPend = result;

    return result;
}

bool DbncdDlydMPBttn::begin(const unsigned long int &pollDelayMs){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback funtion to work                
                mpbPollCallback   // DbncdDlydMPBttn::mpbPollCallback   //Callback function
            );
            if (_mpbPollTmrHndl != nullptr){
                tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
                if (tmrModResult == pdPASS)
                    result = true;
            }
        }
    }

    return result;
}

void DbncdDlydMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    DbncdDlydMPBttn *mpbObj = (DbncdDlydMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (pollDelayMs > 0){
        if (!_mpbPollTmrHndl){        
            _mpbPollTmrHndl = xTimerCreate(
                _mpbPollTmrName,  //Timer name
                pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
                pdTRUE,     //Autoreload true
                this,       //TimerID: data passed to the callback function to work                
                mpbPollCallback   // LtchMPBttn::mpbPollCallback   //Callback function
            );
            if (_mpbPollTmrHndl != nullptr){
                tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
                if (tmrModResult == pdPASS)
                    result = true;
            }
        }
    }

    return result;
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

bool LtchMPBttn::unlatch(){
    if(_isOn){
        _dbncTimerStrt = 0;
        _isPressed = false;  //Not needed as the debounce timer was resetted
        _validPressPend = false;
        _releasePending = false;
        _unlatchPending = false;
        _isOn = false;
        _outputsChange = true;
    }

   return _isOn;
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

void LtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){    
    LtchMPBttn *mpbObj = (LtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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
    if(_srvcTime < _MinSrvcTime)    //Best practice would impose failing the constructor (throwing an exeption or building a "zombie" object)
        _srvcTime = _MinSrvcTime;    //this tolerant approach taken for developers benefit, but object will be no faithful to the instantiation parameters

}

const unsigned long int TmLtchMPBttn::getSvcTime() const{

    return _srvcTime;
}

bool TmLtchMPBttn::setSvcTime(const unsigned long int &newSvcTime){
    bool result {false};

    if (newSvcTime > _MinSrvcTime){  //The minimum activation time must be kept
        _srvcTime = newSvcTime;
        result = true;
    }

    return result;
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

void TmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    TmLtchMPBttn *mpbObj = (TmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work
            mpbPollCallback //TmLtchMPBttn::mpbPollCallback   //Callback function
        );
        if (_mpbPollTmrHndl != nullptr){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

bool TmLtchMPBttn::setTmerRstbl(const bool &newIsRstbl){
    if(_tmRstbl != newIsRstbl)
        _tmRstbl = newIsRstbl;

    return _tmRstbl;
}

//=========================================================================> Class methods delimiter

HntdTmLtchMPBttn::HntdTmLtchMPBttn(const uint8_t &mpbttnPin, const unsigned long int &actTime, const unsigned int &wrnngPrctg, const bool &pulledUp, const bool &typeNO, const unsigned long int &dbncTimeOrigSett, const unsigned long int &strtDelay)
:TmLtchMPBttn(mpbttnPin, actTime, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _wrnngPrctg{wrnngPrctg}
{
    _wrnngMs = (_srvcTime * _wrnngPrctg) / 100;   
}

bool HntdTmLtchMPBttn::begin(const unsigned long int &pollDelayMs){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work
            mpbPollCallback //HntdTmLtchMPBttn::mpbPollCallback   //Callback function
        );
        if (_mpbPollTmrHndl != nullptr){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

const bool HntdTmLtchMPBttn::getPilotOn() const{

    return _pilotOn;
}

const bool HntdTmLtchMPBttn::getWrnngOn() const{
    
    return _wrnngOn;
}

bool HntdTmLtchMPBttn::setSvcTime(const unsigned long int &newSvcTime){
    bool result {true};

    if (newSvcTime != _srvcTime){
        result = TmLtchMPBttn::setSvcTime(newSvcTime);
        if (result)
            _wrnngMs = (_srvcTime * _wrnngPrctg) / 100;  //If the _srvcTime was changed, the _wrnngMs must be updated as it's a percentage of the first 
    }

    return result;
}

bool HntdTmLtchMPBttn::setKeepPilot(const bool &newKeepPilot){
    if(_keepPilot != newKeepPilot)
        _keepPilot = newKeepPilot;
    
    return _keepPilot;
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
        else{
            if(_wrnngOn == true){
                _wrnngOn = false;
                _outputsChange = true;                    
            }
        }
    }
    
    return _wrnngOn;
}

void HntdTmLtchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    HntdTmLtchMPBttn *mpbObj = (HntdTmLtchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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

XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  DbncdDlydMPBttn* unLtchBttn,
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay), _unLtchBttn{unLtchBttn}
{

}
XtrnUnltchMPBttn::XtrnUnltchMPBttn(const uint8_t &mpbttnPin,  
        const bool &pulledUp,  const bool &typeNO,  const unsigned long int &dbncTimeOrigSett,  const unsigned long int &strtDelay)
:LtchMPBttn(mpbttnPin, pulledUp, typeNO, dbncTimeOrigSett, strtDelay)
{

}

bool XtrnUnltchMPBttn::unlatch(){
    if(_isOn){
        _dbncTimerStrt = 0;
        _isPressed = false;  //Not needed as the debounce timer was resetted
        _validPressPend = false;
        // _releasePending = false;     //not needed in this subclass
        _unlatchPending = false;
        _isOn = false;
        _outputsChange = true;
    }

   return _isOn;
}

bool XtrnUnltchMPBttn::updIsOn(){
    if (_unlatchPending){
        if (_isOn){
            _isOn = false;
            _outputsChange = true;
        }
        _unlatchPending = false;
        _validPressPend = false;
    }
    else if (_validPressPend){
        if (!_isOn){
            _isOn = true;
            _outputsChange = true;
        }
        _validPressPend = false;
        _unlatchPending = false;
    }

    return _isOn;
}

bool XtrnUnltchMPBttn::begin(const unsigned long int &pollDelayMs){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work
            mpbPollCallback //XtrnUnltchMPBttn::mpbPollCallback   //Callback function
        );
        if (_mpbPollTmrHndl != nullptr){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

bool XtrnUnltchMPBttn::updUnlatchPend(){
    if(_isOn){        
        if(_unLtchBttn != nullptr){
            if (_unLtchBttn->getIsOn()){
                _unlatchPending = true;
                _validPressPend = false;
            }
        }
    }

    return _unlatchPending;
}

void XtrnUnltchMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    XtrnUnltchMPBttn *mpbObj = (XtrnUnltchMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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

void VdblMPBttn::clrStatus(){
    /*To Resume operation after a pause() without risking generating false "Valid presses" and "On" situations,
    several attributes must be resetted to "Start" values
    */
    DbncdMPBttn::clrStatus();
    setIsVoided(false);
    _outputsChange = true;

    return;
}

const bool VdblMPBttn::getIsEnabled() const{

    return _isEnabled;
}

const bool VdblMPBttn::getIsOnDisabled() const{

    return _isOnDisabled;
}

const bool VdblMPBttn::getIsVoided() const{

    return _isVoided;
}

bool VdblMPBttn::setIsEnabled(const bool &newEnabledValue){
    if(_isEnabled != newEnabledValue){
        if (!newEnabledValue){  //Changed to !Enabled (i.e. Disabled)
            pause();    //It's pausing the timer that keeps the inputs updated and calculates and updates the output flags... Flags must be updated for the disabled condition
            clrStatus();
            if(_isOnDisabled){  //Set the _isOn flag to expected value 
                if(_isOn == false)
                    _isOn = true;
            }
            else{
                if (_isOn == true)
                    _isOn = false;
            }
            if(getTaskToNotify() != nullptr)
                xTaskNotifyGive(getTaskToNotify());
            setOutputsChange(false);
        }
        else{
            clrStatus();
            resume();   //It's resuming the timer that keeps the inputs updated and calculates and updates the output flags... before this some conditions of timers and flags had to be insured
        }
        _isEnabled = newEnabledValue;
        _outputsChange = true;
    }
    
    return _isEnabled;
}

bool VdblMPBttn::setIsOnDisabled(const bool &newIsOnDisabled){
    if(_isOnDisabled != newIsOnDisabled){
        _isOnDisabled = newIsOnDisabled;
        if(!_isEnabled){
            if(_isOn != _isOnDisabled){
                _isOn = _isOnDisabled;
                _outputsChange = true;
            }
        }
    }

    return _isOnDisabled;
}

bool VdblMPBttn::setIsVoided(const bool &newVoidValue){
    if(_isVoided != newVoidValue){
        _isVoided = newVoidValue;
        _outputsChange = true;
    }

    return _isVoided;
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

bool TmVdblMPBttn::begin(const unsigned long int &pollDelayMs){
    bool result {false};
    BaseType_t tmrModResult {pdFAIL};

    if (!_mpbPollTmrHndl){        
        _mpbPollTmrHndl = xTimerCreate(
            _mpbPollTmrName,  //Timer name
            pdMS_TO_TICKS(pollDelayMs),  //Timer period in ticks
            pdTRUE,     //Autoreload true
            this,       //TimerID: data passed to the callback function to work
            mpbPollCallback //TmVdblMPBttn::mpbPollCallback   //Callback function
        );
        if (_mpbPollTmrHndl != nullptr){
            tmrModResult = xTimerStart(_mpbPollTmrHndl, portMAX_DELAY);
            if (tmrModResult == pdPASS)
                result = true;
        }
    }

    return result;
}

void TmVdblMPBttn::clrStatus(){
    /*To Resume operation after a pause() without risking generating false "Valid presses" and "On" situations,
    several attributes must be resetted to "Start" values
    */

    clrStatus();
    _voidTmrStrt = 0;
    _outputsChange = true;

    return;
}

const unsigned long int TmVdblMPBttn::getVoidTime() const{

    return _voidTime;
}

bool TmVdblMPBttn::setIsEnabled(const bool &newEnabledValue){
    if(_isEnabled != newEnabledValue){
        VdblMPBttn::setIsEnabled(newEnabledValue);

        if (newEnabledValue){  //Changed to Enabled
            clrStatus();
            setIsVoided(true);  //For safety reasons if the mpb was disabled and re-enabled, it is set as voided, so if it was pressed when is was re-enabled there's no risk
                                // of activating somethin unexpectedly. It'll have to be released and then pressed back to intentionally set it to ON.
            resume();   //It's resuming the timer that keeps the inputs updated and calculates and updates the output flags... before this some conditions of timers and flags had to be insured
        }
        _isEnabled = newEnabledValue;
        _outputsChange = true;
    }
    
    return _isEnabled;
}

bool TmVdblMPBttn::setIsVoided(const bool &newVoidValue){
    if(newVoidValue){
        _voidTmrStrt = (xTaskGetTickCount() / portTICK_RATE_MS) - (_voidTime + 1);
    }
    if(_isVoided != newVoidValue){
        _outputsChange = true;
        _isVoided = newVoidValue;
    }

    return _isVoided;
}

bool TmVdblMPBttn::setVoidTime(const unsigned long int &newVoidTime){
    bool result{false};

    if(newVoidTime > 0){
        _voidTime = newVoidTime;
        result = true;
    }
    
    return result;
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

bool TmVdblMPBttn::updIsPressed(){
    
    return DbncdDlydMPBttn::updIsPressed();
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

void TmVdblMPBttn::mpbPollCallback(TimerHandle_t mpbTmrCbArg){
    TmVdblMPBttn *mpbObj = (TmVdblMPBttn*)pvTimerGetTimerID(mpbTmrCbArg);
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
