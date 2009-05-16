//for test
#define __DZ_TEST   1

#include "main.h"

#if __DZ_TEST
//#   include <string>
//#   include <ctime>
//#   include <common/Logger.h>
#   include <common/Threads.h>
//#   include <common/Tools.h>
//#   include <common/DataStream.h>
//#   include <common/LockInt.h>
//#   include <common/LockQueue.h>
//#   include <common/RingBuffer.h>
//#   include <common/Epoll.h>
//#   include <common/Poll.h>
//#   include <common/FdMap.h>
#   include <common/Mutex.h>
//#   include <server/frame/CmdSock.h>
//#   include <common/List.h>
//#   include <common/Tree.h>
#   include <common/Semaphore.h>

using namespace NS_SERVER;

#endif

int main(int argc,const char ** argv)
{
#if __DZ_TEST
    INIT_LOGGER(0);
    U32 now = 0;
    if(argc < 2)
        now = time(0);
    else
        now = atoi(argv[1]);
    cout<<Tools::TimeString(now)<<endl;
#else
    //默认服务器配置文件
    const char * serverconf = DEFAULT_CONF_FILE;
    //默认日志配置文件
    const char * logconf = DEFAULT_LOG_CONF_FILE;
    //extract cmd args
    programName = Tools::ProgramName(argv[0]);
    for(int i = 1;i < argc;++i){
        const char * ret = 0;
        if((Tools::ExtractArg(argv[i],"-h",ret) ||
            Tools::ExtractArg(argv[i],"?",ret) ||
            Tools::ExtractArg(argv[i],"--help",ret)) && !ret)
        {
            printUsage();
            return 0;
        }else if(Tools::ExtractArg(argv[i],"-v",ret) && !ret){
            cout<<versionInfo()<<endl;
            return 0;
        }else if(Tools::ExtractArg(argv[i],"-c=",ret) && ret){
            serverconf = ret;
        }else if(Tools::ExtractArg(argv[i],"-l=",ret) && ret){
            logconf = ret;
        }
    }
    //init logger
    INIT_LOGGER(logconf);
    //init main server
    mainServer = new CMainServer;
    mainServer->Init(serverconf);
    //register signals
    registerSignal();
    //start service
    mainServer->StartServer();
    //cleanup
    delete mainServer;
#endif  //__DZ_TEST
    return 0;
}
