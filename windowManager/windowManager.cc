#include <cstdlib>
#include <iostream>
#include <screen/screen.h>
#include <pthread.h>

screen_context_t    mScreenCtx;
screen_event_t      mScreen_ev;      /* handle used to pop events from our queue */
screen_window_t g_win = NULL;
#define DBG_DEBUG(ctxn, format, args...) do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_INFO(ctxn, format, args...)  do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_WARN(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_ERROR(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_FATAL(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)
#define DBG_DEFAULT(ctxn, format, args...)   do {fprintf(stderr, format, ##args);fprintf(stderr,"\n");}while(0)

void *ListenEventsFromWinow(void *)
{
    int val;
    int rc;

    rc = screen_create_event(&mScreen_ev);
    if(rc)
    {
        DBG_ERROR(CMMLS, "[LM]CLayerSwitch: screen_create_event failed(rc = %d).", rc);
        //return false;
    }

    while(true)
    {
        do{
            screen_get_event(mScreenCtx, mScreen_ev, -1); //wait for the events by blocking way

            rc = screen_get_event_property_iv(mScreen_ev, SCREEN_PROPERTY_TYPE, &val);
            if(rc < 0)
            {
                DBG_ERROR(CMMLS, "[LM]CLayerSwitch:screen_get_event_property_iv "
                                 "SCREEN_PROPERTY_TYPE failed(%d).", rc);
                //continue;
            }


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
                       char id[256] = {0};
                       screen_get_window_property_cv( win,
                                                                SCREEN_PROPERTY_ID_STRING,
                                                                256,
                                                                id );

                       fprintf(stderr, "id is  %s\n", id);
                       if (strcmp(id, "video_window") == 0)
                       {
//                            int screen_val = 15;
//                           screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
//                           if (screen_set_window_property_iv( win,
//                                                              SCREEN_PROPERTY_ZORDER,
//                                                              &screen_val ) != 0) {
//                               fprintf(stderr, "Error setting z-order of video window: %s\n",
//                                       strerror(errno));
//                               return EXIT_FAILURE;
//                           }

                           // Set the video window to be visible.
//                           screen_val = 0;
//                           screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
//                           if (screen_set_window_property_iv( win,
//                                                              SCREEN_PROPERTY_VISIBLE,
//                                                              &screen_val) != 0 ) {
//                               fprintf(stderr, "Error making window visible: %s\n",
//                                       strerror(errno));
//                               return EXIT_FAILURE;
//                           }
//                            DBG_INFO(CMMLS, "set some arttribute SCREEN_PROPERTY_WINDOW success");
                           g_win = win;
                           DBG_INFO(CMMLS, "get video_window");
                       }
                       DBG_INFO(CMMLS, "Create set some arttribute SCREEN_PROPERTY_WINDOW success");
                    }

                    if(NULL == g_win)
                    {
                       DBG_WARN(CMMLS, "[MM]CLayerSwitch:screen_get_event_property_pv "
                                       "SCREEN_PROPERTY_WINDOW null window(win: %p).", g_win);
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
                    else
                    {
//                       char id[256] = {0};
//                       screen_get_window_property_cv( win,
//                                                                SCREEN_PROPERTY_ID_STRING,
//                                                                256,
//                                                                id );
//
//                       fprintf(stderr, "id is  %s\n", id);
//                       if (strcmp(id, "appwindow") == 0)
//                       {
//                            int screen_val = 15;
//                           screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
//                           if (screen_set_window_property_iv( win,
//                                                              SCREEN_PROPERTY_ZORDER,
//                                                              &screen_val ) != 0) {
//                               fprintf(stderr, "Error setting z-order of video window: %s\n",
//                                       strerror(errno));
//                               return EXIT_FAILURE;
//                           }
//
//                           // Set the video window to be visible.
//                           screen_val = 0;
//                           screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
//                           if (screen_set_window_property_iv( win,
//                                                              SCREEN_PROPERTY_VISIBLE,
//                                                              &screen_val) != 0 ) {
//                               fprintf(stderr, "Error making window visible: %s\n",
//                                       strerror(errno));
//                               return EXIT_FAILURE;
//                           }
                            DBG_INFO(CMMLS, "POST set some arttribute SCREEN_PROPERTY_WINDOW success");
//                       }
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
                    else {
                        DBG_INFO(CMMLS, "SCREEN_EVENT_CLOSE success");
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
            default:
                break;
            }

        }while(SCREEN_EVENT_NONE != val);
    }
    //Destory the screen object
    screen_destroy_event(mScreen_ev);
    return NULL;
}

bool InitScreen()
{
    int rc = 0;
    screen_context_t ctx;

    if(mScreenCtx)
    {
        return true;
    }

    rc = screen_create_context(&ctx, SCREEN_WINDOW_MANAGER_CONTEXT);
    if(rc)
    {
        DBG_ERROR(CMMLS, "[LM]CLayerSwitch: create the context failed.");
        mScreenCtx = NULL;
        return false;
    }

    mScreenCtx = ctx;
    DBG_INFO(CMMLS, "[LM]CLayerSwitch: init screen succeed.");
    return true;
}

int main(int argc, char *argv[]) {
    pthread_t id_1;
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	InitScreen();
    int ret = pthread_create(&id_1, NULL, ListenEventsFromWinow, NULL);
    if(ret != 0)
    {
        return -1;
    }
	//ListenEventsFromWinow();
    char c;
	while(1)
	{
	    c = getchar();
	    switch (c)
	    {
	    case 't':
	        if (g_win != NULL)
	        {
                int screen_val = 1;
                //screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
                if (screen_set_window_property_iv( g_win,
                                                   SCREEN_PROPERTY_VISIBLE,
                                                   &screen_val) != 0 )
                {
                    fprintf(stderr, "Error making window visible: %s\n",
                    strerror(errno));
                }
                else
                {
                    screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
                    fprintf(stderr, "window set visible");
                }
	        }
	        break;
	    case 'f':
	        if (g_win != NULL)
	        {
                int screen_val = 0;
                //screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
                if (screen_set_window_property_iv( g_win,
                                                   SCREEN_PROPERTY_VISIBLE,
                                                   &screen_val) != 0 )
                {
                    fprintf(stderr, "Error making window visible: %s\n",
                    strerror(errno));
                }
                else
                {
                    screen_flush_context(mScreenCtx, SCREEN_WAIT_IDLE);
                    fprintf(stderr, "window set invisible");
                }
	        }
	        break;
        default:
            break;
	    }
	}
	return EXIT_SUCCESS;
}
