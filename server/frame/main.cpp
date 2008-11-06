//for test
#define __DZ_TEST   1

#include "main.h"

#if __DZ_TEST
//#   include <string>
//#   include <ctime>
//#   include <common/Logger.h>
//#   include <common/Threads.h>
#   include <common/Tools.h>
#   include <common/DataStream.h>
//#   include <common/LockInt.h>
//#   include <common/LockQueue.h>
//#   include <common/Epoll.h>
//#   include <common/Poll.h>
//#   include <common/FdMap.h>
//#   include <common/Mutex.h>
//#   include <server/frame/CmdSock.h>
//#   include <common/List.h>
//#   include <common/Tree.h>

//CLockInt<int,CSpinLock> gi;
CLockInt<int,CMutex> gi;

void * thread(void * arg)
{
    for(;;){
        if(++gi > 10000000)
            break;
    }
    return 0;
}
#endif

int main(int argc,const char ** argv)
{
#if __DZ_TEST
    COutByteStream ds;
    ds<<U32(1)<<U8(2)<<__DZ_STRING("abc");
    cout<<ds.Tell()<<endl;
    ds<<Manip::insert(4,U32(8));
    cout<<ds.Tell()<<endl;
    __DZ_VECTOR(char) data;
    ds.ExportData(data);
    cout<<"data="<<Tools::DumpHex(data)<<endl;
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
