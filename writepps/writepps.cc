#include <cstdlib>
#include <iostream>
#include <fcntl.h>
#include <string>
#define PPS_MNT_CARPLAY "/pps/qnx/mount/carplay"
#define PPS_MNT_IPOD "/pps/qnx/mount/ipod0"

void writeToPPSCarplay(std::string &content)
{
    int fd = -1;
    std::string file = PPS_MNT_CARPLAY;
    fd = open(file.c_str(), O_WRONLY | O_CREAT);
    if (fd >= 0)
    {
        std::string ppsObj = "[n]@carplay\n";
        ppsObj += content;
        write(fd, ppsObj.c_str(), ppsObj.size());
        close(fd);
    }
}

void ReadPPSIpod(std::string &content)
{
    int fd = -1;
    std::string file = PPS_MNT_IPOD;
    std::string ppsObj = "[n]@ipod0\n";
    char objContent[1024];

    fd = open(file.c_str(), O_RDONLY);
    if (fd >= 0)
    {
        std::string ipodContent = objContent;
        int rNum = read(fd, objContent, 1024);
        if (rNum <= 0)
        {
            return;
        }
        ipodContent = objContent;
        content = ipodContent.substr(ppsObj.size());
        close(fd);
    }
}

int main(int argc, char *argv[]) {
	std::cout << "Welcome to the QNX Momentics IDE" << std::endl;
	std::string content;

	ReadPPSIpod(content);
	writeToPPSCarplay(content);
	return EXIT_SUCCESS;
}
