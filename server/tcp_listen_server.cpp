#include <Logger.h>
#include "frame/Command.h"
#include "frame/HahsEngine.h"

using namespace ServerModel;

#define LISTEN_IP   "127.0.0.1"
#define LISTEN_PORT 12345

int main(int argc,const char ** argv)
{
    INIT_LOGGER("../conf/logger.conf");
    LOCAL_LOGGER(logger, "main");
    CHahsEngine engine;
    //add listen
    CSockAddr addr(LISTEN_IP, LISTEN_PORT);
    if(!addr.IsValid()){
        FATAL("cannot init addr "<<LISTEN_IP<<":"<<LISTEN_PORT);
        return 1;
    }
    CCmdRecvHelper recvHelper;
    engine.AddTcpListen(addr, recvHelper);
    //run
    CHahsEnginParams param;
    param.notifyStackSz_ = 16 << 10;
    param.ioStackSz_ = 16 << 10;
    param.handlerStackSz_ = 16 << 10;
    param.maxFdNum_ = 1023;
    param.epollTimeoutMs_ = 500;
    param.handlerThreadMax_ = 4;
    if(!engine.Run(param)){
        FATAL("engine run failed");
        return false;
    }
    engine.WaitAll();
    return 0;
}
