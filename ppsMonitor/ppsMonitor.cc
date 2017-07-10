#include <cstdlib>
#include <iostream>
#include <fcntl.h>
const int MAX_PPS_BUFF = 1024;

int main(int argc, char *argv[]) {
    int mFd = -1;
    char rBuff[MAX_PPS_BUFF] = {0};
    mFd = open("/pps/qnx/mount/.all?deltadir,wait", O_RDONLY);

    if (mFd > 0)
    {
        while (1)
        {

            int rNum = read(mFd, rBuff, MAX_PPS_BUFF);
            if (rNum <= 0)
            {
                sleep(1);
                std::cout << "read error " <<std::endl;
            }
            else
            {
                std::string notify = rBuff;
                if ("@" == notify.substr(0, 1))
                {
                    std::cout << "already exist device " <<std::endl;
                    //break;
                }
                else if ("+" == notify.substr(0, 1))
                {
                    std::cout << "new device " <<std::endl;
                    //break;
                }
                else if ("-" == notify.substr(0, 1))
                {
                    std::cout << "delete device " <<std::endl;
                    //break;
                }
            }

        }
    }

	return EXIT_SUCCESS;
}
