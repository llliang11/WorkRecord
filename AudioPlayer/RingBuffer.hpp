#ifndef RingBuffer_HPP_
#define RingBuffer_HPP_

#include "CarLifeTypes.hpp"
class RingBuffer
{
public:
    RingBuffer(int size);
    ~RingBuffer();
    int Write(const UInt8 *buffer, int size);
    void Read(UInt8 **buffer, int *size);
    void Reset();
    void FreeBuffer(UInt8 *buffer);

private:
    int mBufferSize;
    UInt8 *mBuffer;
    UInt8 *mPWrite;
    UInt8 *mPRead;
    UInt8 *mPEnd;
    int mLeftSize;
};

#endif /* RingBuffer_HPP_ */
