#include "usbutil.h"
#include "buffers.h"
#include "log.h"

#define USB_PACKET_SIZE 64

extern "C" {
extern bool handleControlRequest(uint8_t);
}

// This is a reference to the last packet read
extern volatile CTRL_TRF_SETUP SetupPkt;
extern UsbDevice USB_DEVICE;

boolean usbCallback(USB_EVENT event, void *pdata, word size) {
    // initial connection up to configure will be handled by the default
    // callback routine.
    USB_DEVICE.device.DefaultCBEventHandler(event, pdata, size);

    switch(event) {
    case EVENT_CONFIGURED:
        debug("USB Configured\r\n");
        USB_DEVICE.configured = true;
        USB_DEVICE.device.EnableEndpoint(USB_DEVICE.inEndpoint,
                USB_IN_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        USB_DEVICE.device.EnableEndpoint(USB_DEVICE.outEndpoint,
                USB_OUT_ENABLED|USB_HANDSHAKE_ENABLED|USB_DISALLOW_SETUP);
        onUsbConfigured();
        break;

    case EVENT_EP0_REQUEST:
        handleControlRequest(SetupPkt.bRequest);
        break;

    default:
        break;
    }
}

void sendControlMessage(uint8_t* data, uint8_t length) {
    USB_DEVICE.device.EP0SendRAMPtr(data, length, USB_EP0_INCLUDE_ZERO);
}

bool waitForHandle(UsbDevice* usbDevice) {
    int i = 0;
    while(usbDevice->device.HandleBusy(usbDevice->deviceToHostHandle)) {
        ++i;
        if(i > 800) {
            // This can get really noisy when running but I want to leave it in
            // because it' useful to enable when debugging.
            // debug("USB most likely not connected or at least not requesting "
                    // "IN transfers - bailing out of handle waiting\r\n");
            return false;
        }
    }
    return true;
}

void processUsbSendQueue(UsbDevice* usbDevice) {
    while(usbDevice->configured &&
            !QUEUE_EMPTY(uint8_t, &usbDevice->sendQueue)) {
        // Make sure the USB write is 100% complete before messing with this buffer
        // after we copy the message into it - the Microchip library doesn't copy
        // the data to its own internal buffer. See #171 for background on this
        // issue.
        if(!waitForHandle(usbDevice)) {
            return;
        }

        int byteCount = 0;
        while(!QUEUE_EMPTY(uint8_t, &usbDevice->sendQueue) && byteCount < 64) {
            usbDevice->sendBuffer[byteCount++] = QUEUE_POP(uint8_t, &usbDevice->sendQueue);
        }

        int nextByteIndex = 0;
        while(nextByteIndex < byteCount) {
            if(!waitForHandle(usbDevice)) {
                return;
            }
            int bytesToTransfer = min(USB_PACKET_SIZE, byteCount - nextByteIndex);
            usbDevice->deviceToHostHandle = usbDevice->device.GenWrite(
                    usbDevice->inEndpoint, &usbDevice->sendBuffer[nextByteIndex], bytesToTransfer);
            nextByteIndex += bytesToTransfer;
        }
    }
}

void initializeUsb(UsbDevice* usbDevice) {
    initializeUsbCommon(usbDevice);
    usbDevice->device = USBDevice(usbCallback);
    usbDevice->device.InitializeSystem(false);
    debug("Done.\r\n");
}

void armForRead(UsbDevice* usbDevice, char* buffer) {
    buffer[0] = 0;
    usbDevice->hostToDeviceHandle = usbDevice->device.GenRead(
            usbDevice->outEndpoint, (uint8_t*)buffer, usbDevice->outEndpointSize);
}

void readFromHost(UsbDevice* usbDevice, bool (*callback)(uint8_t*)) {
    if(!usbDevice->device.HandleBusy(usbDevice->hostToDeviceHandle)) {
        if(usbDevice->receiveBuffer[0] != NULL) {
            for(int i = 0; i < usbDevice->outEndpointSize; i++) {
                if(!QUEUE_PUSH(uint8_t, &usbDevice->receiveQueue,
                            usbDevice->receiveBuffer[i])) {
                    debug("Dropped write from host -- queue is full\r\n");
                }
            }
            processQueue(&usbDevice->receiveQueue, callback);
        }
        armForRead(usbDevice, usbDevice->receiveBuffer);
    }
}
