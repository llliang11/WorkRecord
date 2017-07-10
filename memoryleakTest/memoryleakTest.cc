#include <cstdlib>
#include <iostream>
#include <stdio.h>
#include "common/Trace.hpp"
#include "common/CTimer.hpp"
using namespace one;

DBG_DEF_APP(TIME, timer test);
DBG_DEF_SCOPE(MAIN, timer main);

static int i = 0;

class TestCallback : public ITimerCallback
{
public:
   virtual void timerHandler( CTimer* pTimer, void* pUser );
};

void TestCallback::timerHandler(CTimer* pTimer, void* pUser)
{
    std::cout << "timer call back" << i++ <<std::endl;
}

int main(int argc, char ** argv)
{
    std::cout << "hello timer" <<std::endl;
    CTimer* testTimer = new CTimer(500, 500);
    TestCallback testCallback;
    //testTimer->start(&testCallback, NULL, 5);
    //delete testTimer;
    testTimer->stop();
    int i = 0;
    int flag = 0;
    while (1)
    {
        i++;
//        if (i > 7)
//        {
//            testTimer->stop();
//        }

        if (i > 5 && flag == 0)
        {
            testTimer->start(&testCallback, NULL, 5);
            flag = 1;
            //delete testTimer;
        }
        sleep(1);
    }
}
