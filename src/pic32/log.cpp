#include "log.h"
#include "HardwareSerial.h"
#include <stdio.h>
#include <stdarg.h>

//extern HardwareSerial Serial2;
extern HardwareSerial Serial;
#define SERIAL  Serial

void debugNoNewline(const char* format, ...) {
#ifdef __DEBUG__
    va_list args;
    va_start(args, format);

    char buffer[MAX_LOG_LINE_LENGTH];
    vsnprintf(buffer, MAX_LOG_LINE_LENGTH, format, args);

    SERIAL.print(buffer);

    va_end(args);
#endif // __DEBUG__
}

void initializeLogging() {
    SERIAL.begin(115200);
}
