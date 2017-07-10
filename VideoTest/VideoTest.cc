#include <cstdlib>
#include <iostream>
#include <mm/renderer.h>
#include <screen/screen.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mm/renderer/events.h>
#include <string>
#include <unistd.h>
using namespace std;
mmr_connection_t *mConnection;
mmr_context_t *mContext;

int32_t mAudioDeviceOutputID;
int32_t mVideoDeviceOutputID;
const char* mContextName;
screen_context_t    screen_context;
screen_window_t     screen_window;
screen_event_t      mScreen_ev;

struct sigevent mSigEvent;
#define DEFAULT_AUDIO_OUT "audio:default"
#define DEFAULT_VIDEO_OUT "screen:"
/*Translate error code to the error message*/
#define DBG_DEBUG(ctxn, format, args...) do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_INFO(ctxn, format, args...)  do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_WARN(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_ERROR(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_FATAL(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_DEFAULT(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
class Code2Message
{
public:
    int code;
    std::string msg;

    Code2Message(int c, const char* m):
    code(c), msg(m){}
};
#define CODE_MSG( code ) Code2Message(code, #code)
#define CODE_MSG_COUNT(codeList) ( sizeof(codeList) / sizeof(codeList[0]) )

#define CODE_MSG( code ) Code2Message(code, #code)
static const  Code2Message errlist[] = {
    CODE_MSG( MMR_ERROR_NONE ),
    CODE_MSG( MMR_ERROR_UNKNOWN ),
    CODE_MSG( MMR_ERROR_INVALID_PARAMETER ),
    CODE_MSG( MMR_ERROR_INVALID_STATE ),
    CODE_MSG( MMR_ERROR_UNSUPPORTED_VALUE ),
    CODE_MSG( MMR_ERROR_UNSUPPORTED_MEDIA_TYPE ),
    CODE_MSG( MMR_ERROR_MEDIA_PROTECTED ),
    CODE_MSG( MMR_ERROR_UNSUPPORTED_OPERATION ),
    CODE_MSG( MMR_ERROR_READ ),
    CODE_MSG( MMR_ERROR_WRITE ),
    CODE_MSG( MMR_ERROR_MEDIA_UNAVAILABLE ),
    CODE_MSG( MMR_ERROR_MEDIA_CORRUPTED ),
    CODE_MSG( MMR_ERROR_OUTPUT_UNAVAILABLE ),
    CODE_MSG( MMR_ERROR_NO_MEMORY ),
    CODE_MSG( MMR_ERROR_RESOURCE_UNAVAILABLE ),
    CODE_MSG( MMR_ERROR_MEDIA_DRM_NO_RIGHTS ),
    CODE_MSG( MMR_ERROR_DRM_CORRUPTED_DATA_STORE ),
    CODE_MSG( MMR_ERROR_DRM_OUTPUT_PROTECTION ),
    CODE_MSG( MMR_ERROR_DRM_OPL_HDMI ),
    CODE_MSG( MMR_ERROR_DRM_OPL_DISPLAYPORT ),
    CODE_MSG( MMR_ERROR_DRM_OPL_DVI ),
    CODE_MSG( MMR_ERROR_DRM_OPL_ANALOG_VIDEO ),
    CODE_MSG( MMR_ERROR_DRM_OPL_ANALOG_AUDIO ),
    CODE_MSG( MMR_ERROR_DRM_OPL_TOSLINK ),
    CODE_MSG( MMR_ERROR_DRM_OPL_SPDIF ),
    CODE_MSG( MMR_ERROR_DRM_OPL_BLUETOOTH ),
    CODE_MSG( MMR_ERROR_DRM_OPL_WIRELESSHD ),
    CODE_MSG( MMR_ERROR_MEDIA_DRM_EXPIRED_LICENSE )
};
#define NERRS ( sizeof(errlist) / sizeof(errlist[0]) )
static const  Code2Message eventTypeList[] = {
    CODE_MSG( MMR_EVENT_NONE ),
    CODE_MSG( MMR_EVENT_ERROR ),
    CODE_MSG( MMR_EVENT_STATE ),
    CODE_MSG( MMR_EVENT_OVERFLOW ),
    CODE_MSG( MMR_EVENT_WARNING ),
    CODE_MSG( MMR_EVENT_STATUS ),
    CODE_MSG( MMR_EVENT_METADATA ),
    CODE_MSG( MMR_EVENT_PLAYLIST ),
    CODE_MSG( MMR_EVENT_INPUT ),
    CODE_MSG( MMR_EVENT_OUTPUT ),
    CODE_MSG( MMR_EVENT_CTXTPAR ),
    CODE_MSG( MMR_EVENT_TRKPAR ),
    CODE_MSG( MMR_EVENT_OTHER )
};
#define EVENT_TYPES_COUNT ( sizeof(eventTypeList) / sizeof(eventTypeList[0]) )

static const  Code2Message eventStatusList[] = {
    CODE_MSG( MMR_STATE_DESTROYED ),
    CODE_MSG( MMR_STATE_IDLE ),
    CODE_MSG( MMR_STATE_STOPPED ),
    CODE_MSG( MMR_STATE_PLAYING )
};
#define EVENT_STATUS_COUNT ( sizeof(eventStatusList) / sizeof(eventStatusList[0]) )
void mmrerror( mmr_context_t *ctxt, const char *errmsg )
{
    const mmr_error_info_t *err = mmr_error_info( ctxt );
    unsigned errcode = err->error_code;
    std::string msg;
    if ( errcode >= NERRS || errlist[errcode].msg.empty()) {
        msg = "bad error code";
    } else {
        msg = errlist[errcode].msg;
    }
    fprintf(stderr, "[error_code=%s][extra_type=%s][extra_value=%lld][extra_text=%s]%s",
        msg.c_str(), err->extra_type, err->extra_value, err->extra_text, errmsg);
}

static char video_device_url[PATH_MAX];
void createWindow()
{

    const char *window_name = "videoWindow";
    char *window_group_name;
    int MAX_WINGRP_NAME_LEN = 49;
    window_group_name = (char *)malloc(MAX_WINGRP_NAME_LEN);
//
//    // Create the video URL for mm-renderer

//
    if (screen_create_context(&screen_context, SCREEN_APPLICATION_CONTEXT) != 0) {
        fprintf( stderr,
        "screen_create_context failed.\n" );
    }
//
    if (screen_create_window(&screen_window, screen_context) != 0) {
        fprintf( stderr,
         "screen_create_window.\n" );
    }
//    // Create a window group.
//    // Pass NULL to generate a unique window group name.
    if (screen_create_window_group(screen_window, NULL) != 0) {
        //return EXIT_FAILURE:
        fprintf( stderr,
         "screen_create_window_group failed.\n" );
    }

    // Get the window group name.
    int rc = screen_get_window_property_cv( screen_window,
                                        SCREEN_PROPERTY_GROUP,
                                        PATH_MAX,
                                        window_group_name );
    //int rc;
    if (rc != 0) {
        fprintf( stderr,
         "screen_get_window_property(SCREEN_PROPERTY_GROUP) failed.\n" );
        //return EXIT_FAILURE;
    }




    rc = snprintf( video_device_url,
                   PATH_MAX,
                   "screen:?winid=%s&wingrp=%s",
                   window_name,
                   window_group_name );
//
    if (rc < 0) {
        fprintf(stderr, "Error building video device URL string\n");
        //return EXIT_FAILURE;
    }
    else if (rc >= PATH_MAX) {
        fprintf(stderr, "Video device URL too long\n");
        //return EXIT_FAILURE;
    }
    fprintf(stderr, "Video device URL is %s\n", video_device_url);
//

    int val = 0;
    int ret = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_SELF_LAYOUT, &val);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set screen usage property.\n");
       //return false;
    }

    // Set up window properties..
     val = SCREEN_USAGE_NATIVE;
     ret = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_USAGE, &val);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set screen usage property.\n");
       //return false;
    }
////
    ret = screen_set_window_property_cv (screen_window, SCREEN_PROPERTY_ID_STRING, (strlen ("appwindow") + 1), "appwindow");
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set screen name property.\n");
       //return false;
    }
//
    int wsize[]={1024, 600};
    ret = screen_set_window_property_iv(screen_window,SCREEN_PROPERTY_SIZE, wsize);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set window size.\n");
       //return false;
    }
//
    int wpos[]={0, 0};
    ret = screen_set_window_property_iv(screen_window,SCREEN_PROPERTY_POSITION, wpos);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set window position.\n");
       //return false;
    }
//
    int format = SCREEN_FORMAT_RGBA8888;
    ret = screen_set_window_property_iv(screen_window, SCREEN_PROPERTY_FORMAT, &format);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to set color format\n");
       //return false;
    }
//
    ret = screen_create_window_buffers(screen_window, 1);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to create window buffers.\n");
       //return false;
    }
//
//     Paint it transparent.
    screen_buffer_t screenBuf;
    ret = screen_get_window_property_pv(screen_window,SCREEN_PROPERTY_RENDER_BUFFERS, (void **) &screenBuf);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to get render buffers.\n");
       //return false;
    }
//
//     So apparently a full transparent window never gets touch events. Make it ever so slightly
//     transparent.
    int fillAttrs[] = { SCREEN_BLIT_COLOR, 0x01000000, SCREEN_BLIT_END };
    ret = screen_fill(screen_context, screenBuf, fillAttrs);
    if (ret != 0)
    {
       fprintf(stderr, "Failed to fill screen. Incorrect setup?\n");
       //return false;
    }
//
////     Invalidate the screen so content shows up. This should really
////     not be hardcoded to 1026x600.
    int rect[] = { 0, 0, 1024, 600};
    ret = screen_post_window(screen_window, screenBuf, 1, rect, 0);
    if (ret != 0)
    {
        fprintf(stderr, "Failed to invalidate screen.\n");
       //return false;
    }
//
//    int screen_val = 300;
//    if (screen_set_window_property_iv( screen_window,
//                                       SCREEN_PROPERTY_ZORDER,
//                                       &screen_val ) != 0) {
//        fprintf(stderr, "Error setting z-order of video window: %s\n",
//                strerror(errno));
//        //return EXIT_FAILURE;
//    }
//    screen_val = 1;
//    screen_flush_context(screen_context, SCREEN_WAIT_IDLE);
//    if (screen_set_window_property_iv( screen_window,
//                                       SCREEN_PROPERTY_VISIBLE,
//                                       &screen_val) != 0 ) {
//        fprintf(stderr, "Error making window visible: %s\n",
//                strerror(errno));
//        //return EXIT_FAILURE;
//    }
    //screen_flush_context(screen_context, SCREEN_WAIT_IDLE);
}

void initialize(const char* pContextName)
{
    //MM2PLAYER_FUNCTION_START();
    const char *mmrname = NULL;
    if ( ( mConnection = mmr_connect( mmrname ) ) == NULL )
    {
        //DBG_ERROR(HMPE, "mmr_connect failed:%s",strerror(errno));
        return ;
    }

    mode_t mode = S_IRWXU|S_IRWXG|S_IRWXO ;
    if ( ( mContext = mmr_context_create( mConnection, pContextName, 0, mode ) ) == NULL )
    {
        //DBG_ERROR(HMPE, "mmr_context_create %s failed:%s", pContextName, strerror(errno));
        return ;
    }

    //output device
    mAudioDeviceOutputID = mmr_output_attach( mContext, DEFAULT_AUDIO_OUT, "audio" );
    if ( mAudioDeviceOutputID < 0 )
    {
        fprintf(stderr, "mmr_output_attach audio error \n");
        //mmrerror( mContext, DEFAULT_AUDIO_OUT );
    }

    createWindow();
    //DBG_INFO(HMPE, "Video device URL is %s\n", video_device_url);
   // DEFAULT_VIDEO_OUT video_device_url
    mVideoDeviceOutputID = mmr_output_attach(mContext, video_device_url, "video");
    if ( mVideoDeviceOutputID < 0 )
    {
        //mmrerror( mContext, DEFAULT_VIDEO_OUT );
        fprintf(stderr, "mmr_output_attach error \n");
    }
}

void ProcessMmrEvent( const mmr_event_t *ev )
{
    fprintf(stderr, "process event ");
    if (ev->type == MMR_EVENT_STATE)
    {
        //State or speed change, other than an error or EOF.
        //tPlayState playState = PLAYSTATE_IDLE;
        switch (ev->state)
        {
        case MMR_STATE_PLAYING:
            if (0 == ev->speed)
            {
                //playState = PLAYSTATE_PAUSED;

                DBG_DEBUG(MPEH, "[%s][%s]Pausing",
                    eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str());
            }
            else
            {
                //playState = PLAYSTATE_PLAYING;

                DBG_DEBUG(MPEH, "[%s][%s]Playing playtime=%s ms",
                    eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
                    ev->pos_str!=NULL?ev->pos_str:"0");
            }
            break;
        case MMR_STATE_STOPPED:
            //playState = PLAYSTATE_STOPPED;

            DBG_DEBUG(MPEH, "[%s][%s]Stopped",
                eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str());
            break;
        case MMR_STATE_IDLE:
            //playState = PLAYSTATE_IDLE;

            DBG_DEBUG(MPEH, "[%s][%s]Idle",
                eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str());
            break;
        case MMR_STATE_DESTROYED:
            DBG_DEBUG(MPEH, "[%s][%s]Destroyed",
                eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str());
            break;
        default:
            DBG_DEBUG(MPEH, "[%s][%s][speed=%d][pos=%s][data=%s]",
                eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
                ev->speed, ev->pos_str!=NULL?ev->pos_str:"NO", ev->data!=NULL?"YES":"NO");
            break;
        }
        //mpListener->OnPlayStateUpdate(playState);
    }
    else if (ev->type == MMR_EVENT_STATUS)
    {
        /*DBG_DEBUG(MPEH, "[%s][%s]Playing playtime=%s ms",
            eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
            ev->pos_str!=NULL?ev->pos_str:"0");*/
//        if (ev->pos_str != NULL)
//        {
//            int pos=0;
//            sscanf(ev->pos_str, "%d", &pos);
//            mpListener->OnPlayPosUpdate(pos);
//        }
    }
    else if (ev->type == MMR_EVENT_METADATA)
    {
        const strm_dict_t *data = ev->data;
        const char* title = strm_dict_find_value(data, "md_title_name");
        const char* artist = strm_dict_find_value(data, "md_title_artist");
        const char* album = strm_dict_find_value(data, "md_title_album");
        const char* genre = strm_dict_find_value(data, "md_title_genre");
        const char* comment = strm_dict_find_value(data, "md_title_comment");
        const char* duration = strm_dict_find_value(data, "md_title_duration");
        DBG_DEBUG(MPEH, "[%s][%s]title=%s, artist=%s, album=%s, genre=%s, coment=%s, duration=%s",
            eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
            title!=NULL?title:"Unknown", artist!=NULL?artist:"Unknown", album!=NULL?album:"Unknown",
            genre!=NULL?genre:"Unknown", comment!=NULL?comment:"Unknown", duration!=NULL?duration:"Unknown");

        string strTitle = (title != NULL) ? title : "";
        string strArtist = (artist != NULL) ? artist : "";
        string strAlbum = (album != NULL) ? album : "";
        string strGenre = (genre != NULL) ? genre : "";
        string strDuration = (duration != NULL) ? duration : "";
        //mpListener->OnMetadataUpdate(strTitle, strArtist, strAlbum, strGenre, strDuration);
    }
    else if (ev->type == MMR_EVENT_ERROR)
    {
        if (MMR_STATE_STOPPED == ev->state){
            DBG_DEBUG(MPEH, "EOF");
            //mpListener->OnSwitchTrackAuto();
        } else {
            DBG_DEBUG(MPEH, "[%s][%s][speed=%d][pos=%s][data=%s]",
                eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
                ev->speed, ev->pos_str!=NULL?ev->pos_str:"NO", ev->data!=NULL?"YES":"NO");
            //mpListener->OnErrorHandle();
        }
    }
    else
    {
        DBG_DEBUG(MPEH, "[%s][%s][speed=%d][pos=%s][data=%s]",
            eventTypeList[ev->type].msg.c_str(), eventStatusList[ev->state].msg.c_str(),
            ev->speed, ev->pos_str!=NULL?ev->pos_str:"NO", ev->data!=NULL?"YES":"NO");
    }
}

int HandleMMrEvent()
{
    fprintf(stderr, "start handle event...");

    int channelID = ChannelCreate(0);
    int connID = ConnectAttach(0, 0, channelID, _NTO_SIDE_CHANNEL, 0);
    SIGEV_PULSE_INIT(&mSigEvent, connID, SIGEV_PULSE_PRIO_INHERIT, _PULSE_CODE_MINAVAIL, 0);
    struct _pulse mPulse;

    while (true)
    {
        //fprintf(stderr, "here  %d", __LINE__);
        /*int mPulseRecID = MsgReceivePulse(channelID, &mPulse, sizeof(mPulse), NULL );
        if (mPulseRecID != 0 || mContext == NULL)
        {
            fprintf(stderr, "NOT A PULSE, mPulseRecID= %d", mPulseRecID);
            continue;
        }*/
        //fprintf(stderr, "here  %d", __LINE__);
        if (true)
        {
            const mmr_event_t *ev = mmr_event_get( mContext );
            if (NULL == ev) {
                DBG_ERROR(MPEH, "mmr event error\n");
                break;
            }
            //fprintf(stderr, "here  %d", __LINE__);
            ProcessMmrEvent(ev);

            int rc = mmr_event_arm(mContext, &mSigEvent);
            if ( rc > 0) {  //event is already available, manually arm
                //fprintf(stderr, "here  %d", __LINE__);
                MsgDeliverEvent(0, &mSigEvent);
            } else if ( rc < 0 ) {
                //fprintf(stderr, "here  %d", __LINE__);
                DBG_DEBUG(MPEH, "mmr_event_arm() error\n");
                break;
            }
        }
        usleep(500000);
    }

    ConnectDetach(connID);
    ChannelDestroy(channelID);

    return 0;
}

bool ListenEventsFromWinow()
{
    int val;
    int rc;

    rc = screen_create_event(&mScreen_ev);
    if(rc)
    {
        DBG_ERROR(CMMLS, "[LM]CLayerSwitch: screen_create_event failed(rc = %d).", rc);
        return false;
    }
    int oldpos[2] = {0,0};

    while(true)
    {
        do{
            screen_get_event(screen_context, mScreen_ev, -1); //wait for the events by blocking way

            rc = screen_get_event_property_iv(mScreen_ev, SCREEN_PROPERTY_TYPE, &val);
            if(rc < 0)
            {
                DBG_ERROR(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_iv "
                                 "SCREEN_PROPERTY_TYPE failed(%d).", rc);
                //continue;
            }

            int pos[2] = {0,0};
            unsigned char cmd [8];
            cmd[0] = 't';
            //screen_get_event_property_iv(event, SCREEN_PROPERTY_TYPE, &val);
            screen_get_event_property_iv(mScreen_ev, SCREEN_PROPERTY_POSITION, pos);

            cmd [2] = (pos [0] & 0x00FF);
            cmd [3] = (pos [0] & 0xFF00) >> 8;

            cmd [4] = (pos [1] & 0x00FF);
            cmd [5] = (pos [1] & 0xFF00) >> 8;

            cmd [6] = 0;
            cmd [7] = 0;

            switch(val)
            {
            case SCREEN_EVENT_CREATE:
                {
                    DBG_INFO(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_iv "
                                    "SCREEN_EVENT_CREATE.");
                    screen_window_t win = NULL;
                    rc = screen_get_event_property_pv(mScreen_ev, SCREEN_PROPERTY_WINDOW,(void **)&win);
                    if(rc < 0)
                    {
                       DBG_ERROR(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_pv "
                                        "SCREEN_PROPERTY_WINDOW failed(%d).", rc);
                       //continue;
                    }
                    else
                    {
                       DBG_INFO(CMMLS, "set some arttribute SCREEN_PROPERTY_WINDOW success");
                    }

                    if(NULL == win)
                    {
                       DBG_WARN(CMMLS, "[MM]CLayerSwitch:screen_get_event_property_pv "
                                       "SCREEN_PROPERTY_WINDOW null window(win: %p).", win);
                       //continue;
                    }
                }
                break;
            case SCREEN_EVENT_POST:
                {
                    DBG_INFO(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_iv "
                                    "SCREEN_EVENT_POST.");
                    screen_window_t win = NULL;
                    rc = screen_get_event_property_pv(mScreen_ev, SCREEN_PROPERTY_WINDOW,(void **)&win);
                    if(rc < 0)
                    {
                       DBG_ERROR(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_pv "
                                        "SCREEN_PROPERTY_WINDOW failed(%d).", rc);
                       //continue;
                    }
                   char id[256] = {0};
                   screen_get_window_property_cv( win,
                                                            SCREEN_PROPERTY_ID_STRING,
                                                            256,
                                                            id );

                   fprintf(stderr, "id is  %s\n", id);
                   if (strcmp(id, "videoWindow") == 0)
                   {
//                        int screen_val = 15;
//                       screen_flush_context(screen_context, SCREEN_WAIT_IDLE);
//                       if (screen_set_window_property_iv( win,
//                                                          SCREEN_PROPERTY_ZORDER,
//                                                          &screen_val ) != 0) {
//                           fprintf(stderr, "Error setting z-order of video window: %s\n",
//                                   strerror(errno));
//                           return EXIT_FAILURE;
//                       }

                       // Set the video window to be visible.
                       int screen_val = 0;
                       //screen_flush_context(screen_context, SCREEN_WAIT_IDLE);
                       if (screen_set_window_property_iv( screen_window,
                                                          SCREEN_PROPERTY_VISIBLE,
                                                          &screen_val) != 0 ) {
                           fprintf(stderr, "Error making window visible: %s\n",
                                   strerror(errno));
                           return EXIT_FAILURE;
                       }
                        DBG_INFO(CMMLS, "set some arttribute SCREEN_PROPERTY_WINDOW success");
                   }

                    if(NULL == win)
                    {
                       DBG_WARN(CMMLS, "[MM]CLayerSwitch:screen_get_event_property_pv "
                                       "SCREEN_PROPERTY_WINDOW null window(win: %p).", win);
                       //continue;
                    }
                    //Add the window to the table list
                }
                break;
            case SCREEN_EVENT_CLOSE:
                {
                    DBG_INFO(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_iv "
                                    "SCREEN_EVENT_CLOSE.");
                    screen_window_t win = NULL;
                    rc = screen_get_event_property_pv(mScreen_ev, SCREEN_PROPERTY_WINDOW,(void **)&win);
                    if(rc < 0)
                    {
                       DBG_ERROR(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_pv "
                                        "SCREEN_PROPERTY_WINDOW failed(%d).", rc);
                       //continue;
                    }
                    //delete the window to the table list
//                    CNotifyWindowEvents *notifyWinEvent = new CNotifyWindowEvents(
//                                                             mScreenCtx,
//                                                             win,
//                                                             SCREEN_EVENT_CLOSE,
//                                                             0,
//                                                             0);
//                    notifyWinEvent->HandleWinEvents();
                    ///TODO: delete the window handle?
                    //screen_destroy_window(win);
                }
                break;
            case SCREEN_EVENT_MTOUCH_TOUCH:
                DBG_INFO( kLogLevelNotice, "Touch Event: SCREEN_EVENT_MTOUCH_TOUCH \n");
                cmd [1] = 1;
                if ((oldpos [0] != pos[0]) || (oldpos [1] != pos[1]))
                {
                   DBG_INFO( kLogLevelNotice, "Touch Event: PosX=%d (%02x %02x), PosY=%d (%02x %02x)\n", pos[0], cmd[3], cmd[2], pos[1], cmd[5], cmd[4]);
                   oldpos [0] = pos[0];
                   oldpos [1] = pos[1];
                   //mq->write(cmd,8);
                }
                break;
            case SCREEN_EVENT_MTOUCH_MOVE:
              cmd [1] = 1;
              DBG_INFO( kLogLevelNotice, "Touch Event: SCREEN_EVENT_MTOUCH_MOVE \n");
              if ((oldpos [0] != pos[0]) || (oldpos [1] != pos[1]))
              {
                 DBG_INFO( kLogLevelNotice, "Touch Event: PosX=%d (%02x %02x), PosY=%d (%02x %02x)\n", pos[0], cmd[3], cmd[2], pos[1], cmd[5], cmd[4]);
                 oldpos [0] = pos[0];
                 oldpos [1] = pos[1];
                 //mq->write(cmd,8);
              }
              break;
            case SCREEN_EVENT_MTOUCH_RELEASE:
                DBG_INFO( kLogLevelNotice, "Touch Event: SCREEN_EVENT_MTOUCH_RELEASE \n");
              cmd [1] = 0;
              //mq->write(cmd,8);
              break;
            default:
                break;
            }

        }while(SCREEN_EVENT_NONE != val);
    }
    //Destory the screen object
    screen_destroy_event(mScreen_ev);
    return true;
}

int main(int argc, char *argv[]) {
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	initialize("video Test");
	int r = mmr_input_attach(mContext, "/fs/usb0/music/AudioVideoFolder/07-H264(1280720_24)MP3(128_44).mp4", "track" );
    if ( r < 0 )
    {
        mmrerror( mContext, "/fs/usb0/music/AudioVideoFolder/07-H264(1280720_24)MP3(128_44).mp4");
        fprintf(stderr, "mmr_input_attach error \n");
    }
    r = mmr_play( mContext );
    if ( r < 0 )
    {
        fprintf(stderr, "play error \n");
    }

    HandleMMrEvent();
    //ListenEventsFromWinow();
	return EXIT_SUCCESS;
}
