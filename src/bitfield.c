#include "bitfield.h"

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
    return (0x1 << numBits) - 1;
}

void reverseBytes(uint64_t* data, int numBytes) {
    uint8_t* bytes = (uint8_t*)data;
    int i;
    for(i=0; i<numBytes/2; ++i) {
        uint8_t temp = bytes[i];
        bytes[i] = bytes[numBytes-i-1];
        bytes[numBytes-i-1] = temp;
    }
}

int startingByte(int startBit) {
    return startBit / 8;
}

int endingByte(int startBit, int numBits) {
    return (startBit + numBits - 1) / 8;
}

uint64_t getBitField(uint64_t data, int startBit, int numBits, uint8_t isLittleEndian) {
    int startByte = startingByte(startBit);
    int endByte = endingByte(startBit, numBits);

    uint8_t* bytes = (uint8_t*)&data;
    uint64_t ret = bytes[startByte];
    if(startByte != endByte) {
        // The lowest byte address contains the most significant bit.
        int i;
        for(i = startByte + 1; i <= endByte; i++) {
            ret = ret << 8;
            ret = ret | bytes[i];
        }
    }

    ret >>= 8 - findEndBit(startBit, numBits);
    ret = ret & bitmask(numBits);
    if(0 == numBits % 8 && isLittleEndian)
        reverseBytes(&ret, numBits / 8);
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
