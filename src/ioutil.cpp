#include "ioutil.h"
#include "log.h"

int lookup(void* key,
        bool (*comparator)(void* key, int index, void* candidates),
        void* candidates, int candidateCount);
        
bool ioSignalComparator(void* name, int index, void* signals) {
    return !strcmp((const char*)name, ((IoSignal*)signals)[index].genericName);
}

IoSignal* lookupSignal(const char* name, IoSignal* signals, int signalCount) {
    bool (*comparator)(void* key, int index, void* candidates) = ioSignalComparator;
    int index = lookup((void*)name, comparator, (void*)signals, signalCount);
    if(index != -1) {
        return &signals[index];
    } else {
        return NULL;
    }
}

/* Private: Determine if the received signal should be sent out and update
 * signal metadata.
 *
 * signal - The signal to look for.
 * send - Will be flipped to false if the signal should not be sent (e.g. the
 *      signal is on a limited send frequency and the timer is not up yet).
 *
 * Returns the float value of the signal decoded from the data.
 */
float preTranslateIo(IoSignal* signal, bool* send) {
    float value = signal->isDigital ? digitalRead(signal->pinNumber)
        : analogRead(signal->pinNumber);

    if(!signal->received || signal->sendClock == signal->sendFrequency - 1) {
        if(send && (!signal->received || signal->sendSame ||
                    value != signal->lastValue)) {
            signal->received = true;
        } else {
            *send = false;
        }
        signal->sendClock = 0;
    } else {
        *send = false;
        ++signal->sendClock;
    }
    return value;
}

/* Private: Update signal metadata after translating and sending.
 *
 * We keep track of the last value of each IO signal (in its raw float form),
 * but we can't update the value until after all translation has happened,
 * in case a custom handler needs to use the value.
 */
void postTranslateIo(IoSignal* signal, float value) {
    signal->lastValue = value;
}

static float passthroughHandler(IoSignal* signal, float value, bool* send, Listener* listener) {
    return value;
}

void translateIoSignal(Listener* listener,
            float (*handler)(IoSignal*, float, bool*, Listener* listener),
            IoSignal* signal) {
    bool send = true;
    float value = preTranslateIo(signal, &send);
    if(send) {
        float processedValue = handler(signal, value, &send, listener);
        //float processedValue = handler(signal, signals, signalCount, value, &send);
        if(send) {
            debug("%s=%.0f", signal->genericName, value);
            sendNumericalMessage(signal->genericName, value, listener);
        }
    }
    postTranslateIo(signal, value);
}
void translateIoSignal(Listener* listener, IoSignal* signal) {
    translateIoSignal(listener, passthroughHandler, signal);
}
