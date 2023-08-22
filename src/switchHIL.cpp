#include <switchHIL.h>

    StrcsTmrSwitch::StrcsTmrSwitch(TmLtchMPBttn *lgcMPB, uint8_t wnngPin, bool keepPilot)
    :_lgcMPB{lgcMPB}, _wnngPin{wnngPin}, _keepPilot{keepPilot}
    {        
        //Set the output pins to the required states
        
        //Set the task to keep the outputs updated and set the function name to the updater function

        
    }

//Create the "keepUpdated" function