#include "RingBuffer.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
RingBuffer::RingBuffer( int size ) 
    :mBufferSize(size)
    ,mBuffer(NULL)
{
    mBuffer = (UInt8 *)malloc(mBufferSize);
    if (mBuffer == NULL)
    {
        printf("ring buffer malloc fail \n");
    }
    mPWrite = mPRead = mBuffer;
    mPEnd = mBuffer + mBufferSize;
    mLeftSize = 0;
}

RingBuffer::~RingBuffer()
{
    if (mBuffer != NULL)
        free(mBuffer);
}

int RingBuffer::Write(const UInt8 *buffer, int size)
{
    int ret = 0;
    if (mPWrite >= mPRead)
    {
        if (mPWrite + size <= mBuffer + mBufferSize)
        {
            memcpy(mPWrite, buffer, size);
            mLeftSize += size;
            mPWrite += size;
            ret = size;
            if (mPWrite > mPEnd)
            {
                mPEnd = mPWrite;
            }
            //printf("Write size %d line %d\n", size, __LINE__);

        }
        else
        {
            if (mPRead > mBuffer + size)
            {
                mPEnd = mPWrite;
                memcpy(mBuffer, buffer, size);
                mPWrite = mBuffer + size;
                mLeftSize += size;
                ret = size;
                //printf("Write size %d line %d\n", size, __LINE__);
                printf("write turn round %d\n", __LINE__);
            }
            else
            {
                printf("out of ring buffer %d\n", __LINE__);
            }
        }
    }
    else
    {
        if (mPWrite + size <= mPRead)
        {
            memcpy(mPWrite, buffer, size);
            mLeftSize += size;
            mPWrite += size;
            ret = size;
            printf("Write size %d line %d\n", size, __LINE__);
        }
        else
        {
            printf("out of ring buffer %d\n", __LINE__);
        }
    }
    return ret;
}

void RingBuffer::Read(UInt8 **buffer, int *size)
{
    int readSize = (*size < mLeftSize) ? *size : mLeftSize;
    //printf("read size %d %d %d\n", *size, mLeftSize, readSize);
    if (readSize > 0)
    {
        if ( mPRead + readSize <= mPEnd)
        {
            *buffer = (UInt8 *)malloc(readSize);
            memcpy(*buffer, mPRead, readSize);
            *size = readSize;
            mPRead += readSize;
            mLeftSize -= readSize;
        }
        else
        {
            *size = mPEnd - mPRead;
            *buffer = (UInt8 *)malloc(*size);
            memcpy(*buffer, mPRead, *size);
            mPRead = mBuffer;
            mLeftSize = mLeftSize - *size;
            printf("read turn round %d\n", __LINE__);
        }
    }
    else
    {
        *buffer = NULL;
        *size = 0;
    }
}

void RingBuffer::Reset()
{
	mPWrite = mPRead = mBuffer;
	mPEnd = mBuffer + mBufferSize;
	mLeftSize = 0;
}

void RingBuffer::FreeBuffer(UInt8 *buffer)
{
    if (buffer != NULL)
    {
        free(buffer);
    }
}





