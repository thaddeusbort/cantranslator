#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "timer.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "listener.h"
#include "bitfield.h"
#include "canread.h"

void handleVINMessage(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener);
void handleCellBroadcast(int messageId, uint64_t data, CanSignal* signals, int signalCount, Listener* listener);
bool handleRequestForVINCommand(const char* name, cJSON* value, cJSON* event, CanSignal* signals, int signalCount);
void customLoopHandler(void);


#endif // _HANDLERS_H_
