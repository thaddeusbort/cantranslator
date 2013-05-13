#include "handlers.h"
#include "canwrite.h"
#include "signals.h"
#include "log.h"

char mVin[18] = { '\0' };
const char* const VIN = "VIN";

void handleVINMessage(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener) {
    // check if Byte1 is 0x49 and Byte2 is 0-2
    // decode the value and concatenate it until we have the whole VIN
    char* byteData = (char*)&data;
    if(byteData[0] == 0x49) {
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
            debug("VIN: %s, %d, %d", mVin, strcmp(oldVin, mVin), strlen(mVin));
        }
    }
}

void handleCellBroadcast(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener) {
    static uint8_t cellMsgCount[3][63] = {{0}};
    #define SEND_AFTER_X_MSGS 20
    // check
    byte* byteData = (byte*)&data;
    uint8_t cellId = byteData[0] + 1;
    uint8_t packId = (messageId-0x712);
    if(packId > 3 || cellId > 63) return;
    if(cellMsgCount[packId-1][cellId-1] == 0 || ++cellMsgCount[packId-1][cellId-1] >= SEND_AFTER_X_MSGS) {
        cellMsgCount[packId-1][cellId-1] = 1;
        float voltage = (byteData[1] << 8 | byteData[2]) * 0.0001f;
    
        char cellName[25];
        strcpy(cellName, "bms_pack");
        itoa(packId, cellName+strlen(cellName), 10);
        strcat(cellName, "_cell");
        if(cellId < 10) strcat(cellName, "0"); // place holder digit
        itoa(cellId, cellName+strlen(cellName), 10);
        strcat(cellName, "_voltage");
        sendNumericalMessage(cellName, voltage, listener);
    }
}

bool requestVIN() {
    // send message with ID 0x7E0, byte1 09, byte2 02
    if(strlen(mVin) != 17) {
        CanSignal* signals = getSignals();
        int signalCount = getSignalCount();
        CanSignal* signal = lookupSignal("service_id", signals, signalCount);
        if(signal != NULL) {
            return sendCanSignal(signal, cJSON_CreateNumber(0x0902), signals, signalCount);
        }
    }
    return false;
}

void customLoopHandler() {
    static int count = 0;

    if(count++ >= 100000) {
        unsigned long time_ms = systemTimeMs();
        debug("Still alive at:%d", time_ms);
        if(strlen(mVin) != 17)
            requestVIN();
        count = 0;
    }
}