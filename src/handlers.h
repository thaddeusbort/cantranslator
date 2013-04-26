#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "ioutil.h"

float handleTurnSignals(IoSignal* signal, float value, bool* send, Listener* listener);
bool indicatorHandler(CanSignal* signal, CanSignal* signals, int signalCount, float value, bool* send);
void handleVINMessage(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener);
void handleCellBroadcast(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener);
bool handleRequestForVINCommand(const char* name, cJSON* value, cJSON* event, CanSignal* signals, int signalCount);
void customLoopHandler(void);
// callback when usb is configured
void onUsbConfigured();

#endif // _HANDLERS_H_
