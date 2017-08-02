#ifndef AUDIO_PLAYER_HPP_
#define AUDIO_PLAYER_HPP_

#include "RingBuffer.hpp"
#include "SDL/SDL.h"
#include "CarLifeTypes.hpp"
extern "C"
{
#include "libavresample/avresample.h"
#include "libavutil/opt.h"
#include "libavutil/mem.h"
}

enum PlayerState
{
    PLAYER_STATE_ERROR,
    PLAYER_STATE_CLOSED,
    PLAYER_STATE_RUNNING
};

class CAudioPlayer
{
public:
    static CAudioPlayer* GetAudioPlayer(ChannelType type);
    static void FillData(void *udata,Uint8 *stream,int len);

    bool Open(const uint32_t rate, const uint32_t channel, const uint32_t format);
    int WriteAudioData(const uint8_t *data, uint32_t length, uint32_t timestamp);
    void Close();
private:
    CAudioPlayer(const ChannelType channelType);
    ~CAudioPlayer();
    ChannelType mPlayerType;
    PlayerState mPlayerState;

    uint32_t mSampleRate;
    uint32_t mChannel;
    uint32_t mFormat;
    uint32_t mTargetSampleRate;
    uint32_t mTargetChannel;
    uint32_t mTargetFormat;
    RingBuffer * mRingBuffer;
    static RingBuffer *mRingBuffers[3];
    static bool mSdlInitFlag;
    AVAudioResampleContext *mAvr;
};

#endif /* AUDIO_PLAYER_HPP_ */
