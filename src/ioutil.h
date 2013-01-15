#ifndef _IOUTIL_H_
#define _IOUTIL_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "listener.h"
#include "bitfield.h"
#include "queue.h"
#include "cJSON.h"

//#define BUS_MEMORY_BUFFER_SIZE 2 * 8 * 16

/* Public: An IO signal to decode and output over USB.
 *
 * genericName - The name of the signal to be output over USB.
 * isDigital   - Whether a signal is a digital input or an analog intput.
 * sendFrequency - How often to pass along this message when received. To
 *              process every value, set this to 0.
 * sendSame    - If true, will re-send even if the value hasn't changed.
 * received    - mark true if this signal has ever been received.
 * lastValue   - The last received value of the signal. Defaults to undefined.
 * sendClock   - An internal counter value, don't use this.
 */
struct IoSignal {
    const char* genericName;
    bool isDigital;
    int sendFrequency;
    bool sendSame;
    bool received;
    int pinNumber;
    float lastValue;
    int sendClock;
};
typedef struct IoSignal IoSignal;

/* Public: Look up the IoSignal representation of a signal based on its generic
 * name. The first result will be returned.
 *
 * name - The generic, OpenXC name of the signal.
 * signals - The list of all signals.
 * signalCount - The length of the signals array.
 *
 * Returns a pointer to the IoSignal if found, otherwise NULL.
 */
IoSignal* lookupSignal(const char* name, IoSignal* signals, int signalCount);


/* Public: Read an IO signal, apply the required
 * transforations and send the result to the listener;
 *
 * listener - The listener device to send the final formatted message on.
 * signal - The details of the signal to decode and forward.
 */
void translateIoSignal(Listener* listener, IoSignal* signal);

#endif // _IOUTIL_H_
