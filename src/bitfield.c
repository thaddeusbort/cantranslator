#include "bitfield.h"
#include <stdbool.h>
#include "log.h"

bool bigEndian() {
    union {
        uint32_t i;
        char c[4];
    } bint = {0x01020304};

    return bint.c[0] == 1;
}

/**
 * Find the ending bit of a bitfield within the final byte.
 *
 * Returns: a bit position from 0 to 7.
 */
int findEndBit(int startBit, int numBits) {
    int endBit = (startBit + numBits) % 8;
    return endBit == 0 ? 8 : endBit;
}

uint64_t bitmask(int numBits) {
    return ((uint64_t)0x1u << numBits) - 1;
}

int startingByte(int startBit) {
    return startBit / 8;
}

int endingByte(int startBit, int numBits) {
    return (startBit + numBits - 1) / 8;
}

uint64_t getBitField(uint64_t data, int startBit, int numBits) {
    int startByte = startingByte(startBit);
    int endByte = endingByte(startBit, numBits);

    uint64_t dataCopy = data;
    if(bigEndian()) {
        dataCopy = __builtin_bswap64(data);
    }
    uint8_t* bytes = (uint8_t*)&dataCopy;
    uint64_t ret = bytes[startByte];
    
    //debugNoNewline("Reading CAN message, data = 0x");
    int i=0;
    //while(i < 8) {
    //    debugNoNewline("%02x ", bytes[i]);
    //    i++;
    //}
    //debugNoNewline("\r\n");
    
    uint8_t isLittleEndian = 0;
    if(startByte != endByte) {
        if(!isLittleEndian) {
            // The lowest byte address contains the most significant bit.
            for(i = startByte + 1; i <= endByte; i++) {
                ret = ret << 8;
                ret = ret | bytes[i];
            }
        } else {
            // do something here for little endian?
        }
    }

    ret >>= 8 - findEndBit(startBit, numBits);
    ret = ret & bitmask(numBits);
    return ret;
}

int64_t getTwosComplement(uint64_t data, int numBits) {
    int64_t retVal = data;
    uint8_t unfilledBits;
    if(numBits <= 64 && (data & (0x1u << (numBits-1)))) { // if the highest bit is set, we need to extend it
        unfilledBits = 64-numBits;
        retVal = data | ~((0x1 << numBits) - 1);
    }
    return retVal;
}

/**
 * TODO it would be nice to have a warning if you call with this a value that
 * won't fit in the number of bits you've specified it should use.
 */
void setBitField(uint64_t* data, uint64_t value, int startBit, int numBits) {
    int shiftDistance = 64 - startBit - numBits;
    value <<= shiftDistance;
    *data &= ~(bitmask(numBits) << shiftDistance);
    *data |= value;
}

uint8_t nthByte(uint64_t source, int byteNum) {
    return (source >> (64 - ((byteNum + 1) * 8))) & 0xFF;
}
