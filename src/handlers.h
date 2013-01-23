#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "ioutil.h"

float handleTurnSignals(IoSignal* signal, float value, bool* send, Listener* listener);
bool indicatorHandler(CanSignal* signal, CanSignal* signals, int signalCount, float value, bool* send);

#endif // _HANDLERS_H_
