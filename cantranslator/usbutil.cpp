#include "usbutil.h"
#include "buffers.h"

// TODO move this up to cantranslator.pde
USB_HANDLE USB_INPUT_HANDLE = 0;

void sendMessage(CanUsbDevice* usbDevice, uint8_t* message, int messageSize) {
#ifdef DEBUG
    Serial.print("sending message: ");
    Serial.println((char*)message);
#endif

    // Make sure the USB write is 100% complete before messing with this buffer
    // after we copy the message into it - the Microchip library doesn't copy
    // the data to its own internal buffer. See #171 for background on this
    // issue.
    int i = 0;
    while(usbDevice->configured &&
            usbDevice->device.HandleBusy(USB_INPUT_HANDLE)) { }

    strncpy(usbDevice->sendBuffer, (char*)message, messageSize);
    usbDevice->sendBuffer[messageSize] = '\n';
    messageSize += 1;

#ifdef BLUETOOTH
    // Serial transfers are really, really slow, so we don't want to send unless
    // explicitly set to do so at compile-time. Alternatively, we could send
    // on serial whenever we detect below that we're probably not connected to a
    // USB device, but explicit is better than implicit.
    usbDevice->serial->device->write((const uint8_t*)usbDevice->sendBuffer, messageSize);
#else
    int nextByteIndex = 0;
    while(nextByteIndex < messageSize) {
        while(usbDevice->device.HandleBusy(USB_INPUT_HANDLE));
        int bytesToTransfer = min(USB_PACKET_SIZE,
                messageSize - nextByteIndex);
        uint8_t* currentByte = (uint8_t*)(usbDevice->sendBuffer
                + nextByteIndex);
        USB_INPUT_HANDLE = usbDevice->device.GenWrite(usbDevice->endpoint,
                currentByte, bytesToTransfer);
        nextByteIndex += bytesToTransfer;
    }
#endif
}

void initializeUsb(CanUsbDevice* usbDevice) {
    Serial.print("Initializing USB.....");
    usbDevice->device.InitializeSystem(false);
    Serial.println("Done.");
}

USB_HANDLE armForRead(CanUsbDevice* usbDevice, char* buffer) {
    buffer[0] = 0;
    return usbDevice->device.GenRead(usbDevice->endpoint, (uint8_t*)buffer,
            usbDevice->endpointSize);
}

USB_HANDLE readFromHost(CanUsbDevice* usbDevice, USB_HANDLE handle,
        bool (*callback)(char*)) {
    if(!usbDevice->device.HandleBusy(handle)) {
        // TODO see #569
        delay(500);
        if(usbDevice->receiveBuffer[0] != NULL) {
            strncpy((char*)(usbDevice->packetBuffer +
                        usbDevice->packetBufferIndex), usbDevice->receiveBuffer,
                        ENDPOINT_SIZE);
            usbDevice->packetBufferIndex += ENDPOINT_SIZE;
            processBuffer(usbDevice->packetBuffer,
                &usbDevice->packetBufferIndex, PACKET_BUFFER_SIZE, callback);
        }
        return armForRead(usbDevice, usbDevice->receiveBuffer);
    }
    return handle;
}
