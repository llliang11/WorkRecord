#include "CAudioPlayer.hpp"
 #include <unistd.h>
 
#define DBG_DEBUG(ctxn, format, args...) do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_INFO(ctxn, format, args...)  do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_WARN(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_ERROR(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_FATAL(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_DEFAULT(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)

RingBuffer * CAudioPlayer::mRingBuffers[3] = {NULL};
bool CAudioPlayer::mSdlInitFlag = false;
#define RING_BUF_SIZE 10*1024*1024
CAudioPlayer* CAudioPlayer::GetAudioPlayer( ChannelType type )
{
    switch (type)
    {
    case CHANNEL_TYPE_MEDIA:
        static CAudioPlayer *mediaPlayer = NULL;
        if (mediaPlayer == NULL)
        {
            mediaPlayer = new CAudioPlayer(CHANNEL_TYPE_MEDIA);
        }
        return mediaPlayer;
    case CHANNEL_TYPE_TTS:
        static CAudioPlayer *ttsPlayer = NULL;
        if (ttsPlayer == NULL)
        {
            ttsPlayer = new CAudioPlayer(CHANNEL_TYPE_TTS);
        }
        return ttsPlayer;
    case CHANNEL_TYPE_VR:
        static CAudioPlayer *vrPlayer = NULL;
        if (vrPlayer == NULL)
        {
            vrPlayer = new CAudioPlayer(CHANNEL_TYPE_VR);
        }
        return vrPlayer;
    default:
       // DBG_DEBUG(AUDP, "Get Audio Player failer for channle=%s", toString(type).c_str());
       return NULL;
    }

}

void CAudioPlayer::FillData(void *udata,Uint8 *stream,int len)
{
    RingBuffer **buffers = (RingBuffer **)udata;
    uint8_t *audio_pos = NULL;
    int audio_len = len;

    for(int i = 0; i < 3; i++)
    {
        RingBuffer *buffer = buffers[i];
        if (buffer != NULL)
        {
            audio_len = len;
            buffer->Read(&audio_pos, &audio_len);
            //printf( "buffer %d fill data datalen is %d\n", i, audio_len); 

            if(audio_len==0)
                continue;
            SDL_MixAudio(stream, audio_pos, audio_len, SDL_MIX_MAXVOLUME);
            buffer->FreeBuffer(audio_pos);
        }
    }
}

CAudioPlayer::CAudioPlayer( const ChannelType channelType )
    : mPlayerType(channelType)
    , mPlayerState(PLAYER_STATE_CLOSED)
    , mSampleRate(0)
    , mChannel(0)
    , mFormat(0)
    , mRingBuffer(NULL)
    , mTargetSampleRate(44100)
    , mTargetChannel(2)
    , mTargetFormat(2)
    , mAvr(NULL)

{
}

bool CAudioPlayer::Open(const uint32_t rate, const uint32_t channel, const uint32_t format)
{
    if (mPlayerState != PLAYER_STATE_CLOSED)
    {
        //DBG_ERROR(AUDP, "refuse to open the player for %s as the player state = %d", toString(mPlayerType).c_str(), mPlayerState);
        return -1;
    }

    uint32_t sampleRate = rate;
    uint32_t channelNumber = channel;
    uint32_t sampleFormat = format;
    /*DBG_DEBUG(AUDP, "CAudioPlayer >> Open SampleRate = %d, Channels = %d, SampleBits = %d, type=%s",
        sampleRate, channelNumber, sampleFormat, toString(mPlayerType).c_str());*/
    if (sampleRate == 0 || channelNumber == 0 || sampleFormat == 0)
    {
        sampleRate = mSampleRate;
        channelNumber = mChannel;
        sampleFormat = mFormat;
    }
    else
    {
        mSampleRate = sampleRate;
        mChannel = channelNumber;
        mFormat = sampleFormat;
    }

    if (mSampleRate != mTargetSampleRate && mAvr == NULL)
    {
        mAvr = avresample_alloc_context();
		if (mPlayerType == CHANNEL_TYPE_MEDIA)
		{
			av_opt_set_int(mAvr, "in_channel_layout",  AV_CH_LAYOUT_STEREO, 0);
		}
		else{
			av_opt_set_int(mAvr, "in_channel_layout",  AV_CH_LAYOUT_MONO, 0);
		}

        av_opt_set_int(mAvr, "out_channel_layout", AV_CH_LAYOUT_STEREO,  0);
        av_opt_set_int(mAvr, "in_sample_rate",     mSampleRate,                0);
        av_opt_set_int(mAvr, "out_sample_rate",    mTargetSampleRate,                0);
        av_opt_set_int(mAvr, "in_sample_fmt",      AV_SAMPLE_FMT_S16,   0);
        av_opt_set_int(mAvr, "out_sample_fmt",     AV_SAMPLE_FMT_S16,    0);
        if (0 != avresample_open(mAvr))
        {
            printf("avresample_open fail \n");
        }
    }


    if (mRingBuffer == NULL)
    {
        mRingBuffer = new  RingBuffer(RING_BUF_SIZE);
    }
    else
    {
        mRingBuffer->Reset();
    }

    if (mPlayerType == CHANNEL_TYPE_MEDIA)
    {
        mRingBuffers[0] = mRingBuffer;
    }
    else if (mPlayerType == CHANNEL_TYPE_TTS)
    {
        mRingBuffers[1] = mRingBuffer;
    }
    else if (mPlayerType == CHANNEL_TYPE_VR)
    {
        mRingBuffers[2] = mRingBuffer;
    }
    
    if (mSdlInitFlag == false)
    {
        /*if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO | SDL_INIT_TIMER)) {  
            printf( "Could not initialize SDL - %s\n", SDL_GetError()); 
            return false;
        }*/
        //SDL_AudioSpec
        SDL_AudioSpec wanted_spec;
        wanted_spec.freq = mTargetSampleRate; 
        wanted_spec.format = AUDIO_S16SYS; 
        wanted_spec.channels = mTargetChannel; 
        wanted_spec.silence = 0; 
        wanted_spec.samples = 4096; 
        wanted_spec.callback = CAudioPlayer::FillData; 
        wanted_spec.userdata = mRingBuffers; 

        if (SDL_OpenAudio(&wanted_spec, NULL)<0)
        { 
            printf("can't open audio.\n"); 
        }
        SDL_PauseAudio(0);
        printf("start play  audio.\n"); 
        mSdlInitFlag = true;
    }

    mPlayerState = PLAYER_STATE_RUNNING;
    return true;
}

CAudioPlayer::~CAudioPlayer()
{
    Close();
    if (mRingBuffer != NULL)
        delete mRingBuffer;
}


int CAudioPlayer::WriteAudioData( const uint8_t *data, uint32_t length, uint32_t timestamp )
{
    int ret;
    uint8_t *output = NULL;
    uint8_t *resampleData = NULL; 
    int size = length;

    if (mSampleRate != mTargetSampleRate && mAvr != NULL)
    {
        int out_linesize;
	 int in_samples = length / 2;
        int out_samples = avresample_get_out_samples(mAvr, in_samples);
        //printf("in sample %d out sample %d ....\n", in_samples, out_samples);

        av_samples_alloc(&output, &out_linesize, 2, out_samples, AV_SAMPLE_FMT_S16, 0);
        out_samples = avresample_convert(mAvr, &output, out_linesize, out_samples, (uint8_t **)(&data), 0, in_samples);        
        size = out_linesize;
        resampleData = output;
        //printf("in sample %d out sample %d  out_linesize %d....\n", in_samples, out_samples, out_linesize);
    }
    else
    {
        output = (uint8_t *)data;
    }
    
    do
    {
        ret = mRingBuffer->Write(output, size);
        if (ret == 0)
        {
            usleep(500 * 1000);
            //printf("write length is %d ....\n", size);
            //printf("buffer is full ....\n");
        }
    }
    while(ret == 0);

    if (resampleData != NULL)
    {
        av_freep(&resampleData);       
    }
    return 0;
}

void CAudioPlayer::Close()
{
    /*if (mAvr != NULL)
    {
       avresample_close(mAvr);
       avresample_free(&mAvr);
       mAvr = NULL;
    }*/
     //SDL_CloseAudio();//Close SDL
     //SDL_Quit();
    //DBG_DEBUG(AUDP, "CAudioPlayer << Close[%d] type=%s", __LINE__, toString(mPlayerType).c_str());
}
