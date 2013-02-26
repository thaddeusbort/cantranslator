#include "handlers.h"
#include "canwrite.h"
#include "signals.h"
#include "log.h"

char mVin[18] = { '\0' };

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

bool indicatorHandler(CanSignal* signal, CanSignal* signals,
        int signalCount, float value, bool* send) {
    *send = value || signal->lastValue; // send if value is true or it had been true
    return value;
}

void handleVINMessage(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener) {
    // check if Byte1 is 0x49 and Byte2 is 0-2
    // decode the value and concatenate it until we have the whole VIN
    const char* const VIN = "VIN";
    char* byteData = (char*)&data;
    if(byteData[0] == 0x48) {
        char oldVin[18];
        strcpy(oldVin, mVin);
        switch(byteData[1]) {
        case 0:
            memcpy(mVin, byteData+2, 6);
            break;
        case 1:
            memcpy(mVin+6, byteData+2, 6);
            break;
        case 2:
            memcpy(mVin+12, byteData+2, 5);
            break;
        }
        if(strcmp(oldVin, mVin) && strlen(mVin) == 17) {
            sendStringMessage(VIN, mVin, listener);
            debug("VIN: %s, %d, %d\r\n", mVin, strcmp(oldVin, mVin), strlen(mVin));
        }
    }
}

bool requestVIN() {
    // send message with ID 0x7E0, byte1 09, byte2 02
    if(strlen(mVin) != 17) {
        CanSignal* signals = getSignals();
        int signalCount = getSignalCount();
        CanSignal* signal = lookupSignal("vin_request", signals, signalCount);
        if(signal != NULL) {
            return sendCanSignal(signal, cJSON_CreateNumber(0x0902), signals, signalCount);
        }
    }
    return false;
}

void customLoopHandler() {
    static int count = 125000;
    if(++count >= 125000) {
        count = 0;
        requestVIN();
    }
}