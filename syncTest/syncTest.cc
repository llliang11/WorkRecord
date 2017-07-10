#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include "boost/functional/factory.hpp"
#include "boost/smart_ptr.hpp"
#include "boost/function.hpp"
#include "boost/bind.hpp"
#include <mmsync/mmsyncclient.h>
#include <mmsync/interface.h>
#include <mmsync/event.h>
#include <sys/neutrino.h>
#include "common/CThread.hpp"
#include "common/TAsyncExecutor.hpp"
#include "common/Trace.hpp"
#include <backtrace.h>
//#define DBG_INFO(ctxn, format, args...) printf(format, ##args); printf("\n");
#define TIMEOUT_VALUE (500000000)
#define TIMER_INTERVAL (TIMEOUT_VALUE)

DBG_DEF_APP(SYNC, sync test);
DBG_DEF_SCOPE(MMSY, sync test);
using namespace one;

std::string toString(mmsync_event_type_t event)
{
    static std::string evtList[] = {
        "MMSYNC_EVENT_NONE",
        "MMSYNC_EVENT_MS_1PASSCOMPLETE",
        "MMSYNC_EVENT_MS_SYNCCOMPLETE",
        "MMSYNC_EVENT_MS_UPDATE",
        "MMSYNC_EVENT_SHUTDOWN",
        "MMSYNC_EVENT_SHUTDOWN_COMPLETED",
        "MMSYNC_EVENT_MS_SYNCFIRSTFID",
        "MMSYNC_EVENT_SYNC_ERROR",
        "MMSYNC_EVENT_SYNCABORTED",
        "MMSYNC_EVENT_SYNC_SKIPPED",
        "MMSYNC_EVENT_MS_SYNC_STARTED",
        "MMSYNC_EVENT_MS_2PASSCOMPLETE",
        "MMSYNC_EVENT_MS_3PASSCOMPLETE",
        "MMSYNC_EVENT_MS_SYNC_PENDING",
        "MMSYNC_EVENT_MS_SYNC_FOLDER_STARTED",
        "MMSYNC_EVENT_MS_SYNC_FOLDER_COMPLETE",
        "MMSYNC_EVENT_MS_SYNC_PRIORITY_FOLDER_STARTED",
        "MMSYNC_EVENT_MS_SYNC_PRIORITY_FOLDER_COMPLETE",
        "MMSYNC_EVENT_MS_SYNC_FOLDER_CONTENTS_COMPLETE",
        "MMSYNC_EVENT_MS_SYNC_FIRST_EXISTING_FID",
        "MMSYNC_EVENT_BUFFER_TOO_SMALL",
        "MMSYNC_EVENT_MS_SYNC_FOLDER_TRIM_COMPLETE",
        "MMSYNC_EVENT_DB_RESET",
        "MMSYNC_EVENT_PLAYLIST_ENTRIES_UPDATE",
    };
    return evtList[event];
}
class CMmsyncImpl
{
public:
    CMmsyncImpl(){};
    ~CMmsyncImpl(){};

public:
    void StartSync(std::string path);
    void SyncEventLoop();
    void InitMMSync();
    void ProcessSyncEvent(mmsync_event_type_t event, void *pData);

public:
    void On();

private:
    boost::shared_ptr<CThread> mCtrlThread;
    Int32 mChannelID;
    Int32 mConnID;
    mmsync_hdl_t *mSyncHdl;
    struct sigevent mSigEvent;
    struct _pulse mPulse;
    struct itimerspec itime;
};

void CMmsyncImpl::StartSync(std::string path)
{
    std::string relPath;

    //std::cout << "sync db handle path is " << dbPath << " mnt path is " << device.mMntPath << std::endl;
    std::cout << "string path is " << path << std::endl;
    mm_sync_start(mSyncHdl, "/dev/qdb/DT_USB_8CDDECEEE431D61134F9ECCFF8A3E36E", "/fs/usb0", path.c_str(), MMSYNC_OPTION_RECURSIVE, NULL);
    SyncEventLoop();
}

void CMmsyncImpl::InitMMSync()
{
    timer_t timer_id(0);

    mSyncHdl = mm_sync_connect("/dev/mmsync", 0);
    if (NULL == mSyncHdl)
    {
        //DBG_ERROR(MMSY, "mm_sync_connect error ...");
    }

    mChannelID = ChannelCreate(0);
    mConnID = ConnectAttach(0, 0, mChannelID, _NTO_SIDE_CHANNEL, 0);
    SIGEV_PULSE_INIT(&mSigEvent, mConnID, SIGEV_PULSE_PRIO_INHERIT, _PULSE_CODE_MINAVAIL, 0);
    mm_sync_events_register(mSyncHdl, &mSigEvent);

    timer_create(CLOCK_MONOTONIC, &mSigEvent, &timer_id);
    itime.it_value.tv_sec = 0;
    itime.it_value.tv_nsec = TIMEOUT_VALUE;
    itime.it_interval.tv_sec = 0;
    itime.it_interval.tv_nsec = TIMER_INTERVAL;
    timer_settime(timer_id, 0, &itime, NULL);
    int *ptr;
    delete ptr;
}

void CMmsyncImpl::ProcessSyncEvent(mmsync_event_type_t event, void *pData)
{
    DBG_INFO(MMSY, "process event %s", toString(event).c_str());
    switch (event)
    {
        case MMSYNC_EVENT_MS_SYNC_STARTED:
        {
            break;
        }
        case MMSYNC_EVENT_SYNC_ERROR:
        {
            break;
        }
        //case MMSYNC_EVENT_SYNCABORTED:
        case MMSYNC_EVENT_MS_SYNCCOMPLETE:
        {
            break;
        }
        case MMSYNC_EVENT_MS_1PASSCOMPLETE:
        case MMSYNC_EVENT_MS_2PASSCOMPLETE:
        case MMSYNC_EVENT_MS_3PASSCOMPLETE:
        {
            break;
        }
        case MMSYNC_EVENT_MS_SYNC_FIRST_EXISTING_FID:
        case MMSYNC_EVENT_MS_SYNCFIRSTFID:
        {
            break;
        }
        case MMSYNC_EVENT_MS_UPDATE:
        {
            break;
        }
        default:
        {
            DBG_INFO(MMSY, "not deal with this event");
            break;
        }
    }
}

void CMmsyncImpl::SyncEventLoop()
{
    DBG_INFO(MMSY, "+ %s %d", __FUNCTION__, __LINE__);
    mmsync_event_t *mmsync_event = NULL;
    int mPulseRecID;
    while (true)
    {
        mPulseRecID = MsgReceivePulse(mChannelID, &mPulse, sizeof(mPulse), NULL );
        if (mPulseRecID != 0)
        {
            //DBG_ERROR(MMSY, "NOT A PULSE, mPulseRecID= %d", mPulseRecID);
            DBG_INFO(MMSY, "NOT A PULSE ...");
            continue;
        }

        while (true)
        {
            if (-1 == mm_sync_events_get(mSyncHdl, &mmsync_event))
            {
                DBG_INFO(MMSY, "mm_sync_events_get error ...");
                break;
            }

            if (MMSYNC_EVENT_NONE == mmsync_event->type)
            {
                break;
            }

            ProcessSyncEvent(mmsync_event->type, (void *)mmsync_event->data);

        }
    }
}

void CMmsyncImpl::On()
{
    InitMMSync();

    //mCtrlThread = boost::make_shared<CThread>(" sync ctrl thread ");
    //mCtrlThread->start(this, &CMmsyncImpl::SyncEventLoop);
    char out[1024];
    bt_addr_t pc[16];
    bt_accessor_t acc;
    bt_memmap_t memmap;
    bt_init_accessor(&acc, BT_SELF);
    //bt_init_accessor(&acc, BT_PROCESS, remotepid, remotetid);
    bt_load_memmap(&acc, &memmap);
    bt_sprn_memmap(&memmap, out, sizeof(out));

    for (int i=0; i<10; i++) {
      int cnt=bt_get_backtrace(&acc, pc, sizeof(pc)/sizeof(bt_addr_t));
      bt_sprnf_addrs(&memmap, pc, cnt, "%a\n", out, sizeof(out), 0);
      puts(out);
    }
    bt_unload_memmap(&memmap);
    bt_release_accessor(&acc);
    CREATE_DETACHED_THREAD("sync", &CMmsyncImpl::SyncEventLoop);
}

int main(int argc, char *argv[]) {
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	CMmsyncImpl mmsync;
	mmsync.On();
	//mmsync.StartSync(argv[1]);

	while(1)
	{
	    ;
	}
	return 0;
}
