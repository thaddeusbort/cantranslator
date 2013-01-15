#include "ioutil.h"
#include "log.h"

int lookup(void* key,
        bool (*comparator)(void* key, int index, void* candidates),
        void* candidates, int candidateCount) {
    for(int i = 0; i < candidateCount; i++) {
        if(comparator(key, i, candidates)) {
            return i;
        }
    }
    return -1;
}

bool signalComparator(void* name, int index, void* signals) {
    return !strcmp((const char*)name, ((IoSignal*)signals)[index].genericName);
}

IoSignal* lookupSignal(const char* name, IoSignal* signals, int signalCount,
        bool writable) {
    bool (*comparator)(void* key, int index, void* candidates) = signalComparator;
    if(writable) {
        comparator = writableSignalComparator;
    }
    int index = lookup((void*)name, comparator, (void*)signals, signalCount);
    if(index != -1) {
        return &signals[index];
    } else {
        return NULL;
    }
}

IoSignal* lookupSignal(const char* name, IoSignal* signals, int signalCount) {
    return lookupSignal(name, signals, signalCount, false);
}

void translateIoSignal(Listener* listener, IoSignal* signal) {
    bool send = true;
    float value = preTranslateIo(signal, &send);
    if(send) {
        //float processedValue = handler(signal, signals, signalCount, value, &send);
        sendNumericalMessage(signal->genericName, value, &listener);
    }
    postTranslateIo(signal, value);
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