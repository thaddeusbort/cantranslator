#include "handlers.h"
#include "canwrite.h"


float handleTurnSignals(IoSignal* signal, IoSignal* signals, int signalCount, float value, bool* send) {
    send = FALSE;   // don't send the individual signals only the combined signal
    bool isLeft = signal->genericName.strcmp("turn_signal_left");
    IoSignal* otherTurnSignal = lookupSignal("turn_signal_" + (isLeft ? "right" : "left") , signals, signalCount);
    
    char* status;
    if(value && otherTurnSignal->lastValue)
        status = "BOTH";
    else if(value)
        status = isLeft ? "LEFT" : "RIGHT";
    else if(otherTurnSignal->lastValue)
        status = isLeft ? "RIGHT" : "LEFT";
    else
        status = "OFF";
    
    sendNumericalMessage("turn_signal_status", status, listener);
    return value;   // need to return the value so that it gets saved as the lastValue for the signal
}
