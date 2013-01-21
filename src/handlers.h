#ifndef _HANDLERS_H_
#define _HANDLERS_H_

#include "canread.h"

float handleTurnSignals(CanSignal* signal, CanSignal* signals, int signalCount, float value, bool* send);


#endif // _HANDLERS_H_
