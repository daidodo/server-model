#include <Logger.h>
#include "frame/Command.h"
#include "frame/HahsEngine.h"

using namespace ServerModel;

int main(int argc,const char ** argv)
{
    CHahsEngine engine;
    CSockAddr addr("127.0.0.1", 12345);
    CRecvHelper recvHelper;
    recvHelper.InitRecvSize(5);
    recvHelper.OnDataArrive(CCmdBase::OnDataArrive);
    recvHelper.DecodeCmd(CCmdBase::DecodeCmd);
    recvHelper.ReleaseCmd(CCmdBase::ReleaseCmd);

    engine.AddTcpListen(addr, recvHelper);

    return 0;
}
