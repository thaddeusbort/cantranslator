#include "usbutil.h"
#include "log.h"
#include "canread.h"
#include "ioutil.h"
#include "signals.h"

void initializeUsbCommon(UsbDevice* usbDevice) {
    debug("Initializing USB.....");
    QUEUE_INIT(uint8_t, &usbDevice->sendQueue);
    QUEUE_INIT(uint8_t, &usbDevice->receiveQueue);
    usbDevice->configured = false;
}

void onUsbConfigured() {
    CanSignal* canSignals = getSignals();
    int count = getSignalCount();
    int idx;
    for(idx=0; idx<count; ++idx) {
        canSignals[idx].received = false;
    }

    IoSignal* ioSignals = getIoSignals();
    count = getIoSignalCount();
    for(idx=0; idx<count; ++idx) {
        ioSignals[idx].received = false;
    }
}
