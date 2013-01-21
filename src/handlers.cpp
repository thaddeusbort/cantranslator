#include "handlers.h"
#include "canwrite.h"
#include "signals.h"
#include "log.h"


float handleTurnSignals(IoSignal* signal, float value, bool* send, Listener* listener) {
    *send = false;   // don't send the individual signals only the combined signal
    const char* const RIGHT_NAME = "turn_signal_status";
    const char* const LEFT_NAME = "turn_signal_left";
    bool isLeft = strcmp(LEFT_NAME, signal->genericName) == 0;
    IoSignal* otherTurnSignal;
    otherTurnSignal = lookupSignal(isLeft? RIGHT_NAME: LEFT_NAME, getIoSignals(), getIoSignalCount());
    
    const char* status;
    if(value && otherTurnSignal->lastValue)
        status = "BOTH";
    else if(!value && !otherTurnSignal->lastValue)
        status = "OFF";
    else if((isLeft && value) || !(isLeft || value))
        status = "LEFT";
    else
        status = "RIGHT";

    sendStringMessage(RIGHT_NAME, status, listener);
    return value;   // need to return the value so that it gets saved as the lastValue for the signal
}
