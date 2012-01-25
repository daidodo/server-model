#include <Logger.h>
#include "frame/HahsEngine.h"

using namespace ServerModel;

static __OnDataArriveRet onDataArrive(const __Buffer & buf)
{
    LOCAL_LOGGER(logger, "onDataArrive");
    INFO("recv buf="<<Tools::Dump(buf));

    return __OnDataArriveRet(RR_COMPLETE, 0);
}

int main(int argc,const char ** argv)
{
    CHahsEngine engine;
    CSockAddr addr("127.0.0.1", 12345);
    CRecvHelper recvHelper;
    recvHelper.InitRecvSize(1024);
    recvHelper.OnDataArrive(onDataArrive);

    engine.AddTcpListen(addr, recvHelper);

    return 0;
}
