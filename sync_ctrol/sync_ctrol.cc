#include <cstdlib>
#include <iostream>
#include <mmsync/mmsyncclient.h>
#include <mmsync/interface.h>
#include <sys/neutrino.h>
 #include <unistd.h>

#define success 0
#define failure -1

int main(int argc, char *argv[]) {
	int mChannelID;
	int mConnID;
	mmsync_hdl_t *sync_hdl = NULL;

	struct sigevent event;
	struct _pulse mPulse;
	mmsync_event_t *mmsync_event = NULL;


	sync_hdl = mm_sync_connect("/dev/mmsync", 0);
	if (NULL == sync_hdl)
	{
		std::cout << "mm_sync connect fail ..." << std::endl;
		return EXIT_FAILURE;
	}
	//SIGEV_NONE_INIT(&event);
	mChannelID = ChannelCreate(0);
	mConnID = ConnectAttach(0, 0, mChannelID, _NTO_SIDE_CHANNEL, 0);
	SIGEV_PULSE_INIT(&event, mConnID, SIGEV_PULSE_PRIO_INHERIT, _PULSE_CODE_MINAVAIL, 0);
	mm_sync_events_register(sync_hdl, &event);

	std::cout << "begin start sync ..." << std::endl;
	mm_sync_start(sync_hdl, "/dev/qdb/test_db", "/fs/usb0/", "/", MMSYNC_OPTION_RECURSIVE, NULL);

	while (true)
	{
		int mPulseRecID = MsgReceivePulse(mChannelID, &mPulse, sizeof(mPulse), NULL );
		if (mPulseRecID != 0)
		{
			//DBG_ERROR(("NOT A PULSE, mPulseRecID= %d", mPulseRecID));
			std::cout << "NOT A PULSE, mPulseRecID= " << mPulseRecID <<std::endl;
			continue;
		}
		else
		{
			std::cout << "MsgReceivePulse" << mPulseRecID <<std::endl;
		}
		while (true)
		{
			if (-1 == mm_sync_events_get(sync_hdl, &mmsync_event))
			{
				std::cout << "mm_sync_events_get error ..." << std::endl;
				break;
			}

			if (mmsync_event->type == MMSYNC_EVENT_NONE)
			{
				std::cout << "mmsync_event is NULL ..." << std::endl;
				break;
			}
			else
			{
				std::cout << "mmsync_event type is  ..." << mmsync_event->type << std::endl;
			}

			if (mmsync_event->type == MMSYNC_EVENT_SYNC_ERROR)
			{
				mmsync_sync_error_t *data =  (mmsync_sync_error_t *)mmsync_event->data;
				std::cout << "mmsync error  " << std::endl;
				std::cout << "operation_id is  " << data->type << std::endl;
				std::cout << "error is  " << data->operation_id << std::endl;
				goto End;
			}
			else if (mmsync_event->type == MMSYNC_EVENT_MS_SYNCCOMPLETE)
			{
				std::cout << "mmsync sync completed  " << std::endl;
				goto End;
			}
		}
		//sleep(1);
	}
End:
	ConnectDetach(mConnID);
	ChannelDestroy(mChannelID);
	mm_sync_disconnect(sync_hdl);
	std::cout << "stop sync ..." << std::endl;
	return EXIT_SUCCESS;
}
