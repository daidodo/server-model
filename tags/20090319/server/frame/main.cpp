//for test
#define __DZ_TEST   0

#include "main.h"

#if __DZ_TEST
//#   include <string>
//#   include <ctime>
//#   include <common/Logger.h>
//#   include <common/Threads.h>
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

////typedef __DZ_STRING __Type;
//typedef int __Type;
//
////#define WWH
//
//#ifdef WWH
//typedef pv_queue<__Type> __Que;
//#else
////typedef CLockQueue<__Type> __Que;
//typedef CRingBuf<__Type> __Que;
//#endif
//
//__Que que(1000);
//
//__DZ_STRING randVal()
//{
//    int len = (rand() & 0xF) + 3;
//    __DZ_STRING ret(len,0);
//    for(int i = 0;i < len;++i)
//        ret[i] = (rand() & 63) + 33;
//    return ret;
//}
//
//int randVal(int)
//{
//    return rand();
//}
//
//
//void * thread1(void * arg)
//{
//    const int COUNT = int(arg);
//    for(int j = 0;j < COUNT;++j)
//    for(int i = 0;i < COUNT;){
//#ifdef WWH
//        if(que.push(randVal(i)) < 0){
//#else
//        if(!que.Push(randVal(i))){
//#endif
//            sleep(0);
//        }else
//            ++i;
//    }
//    sleep(-1);
//    return 0;
//}
//
//void * thread2(void * arg)
//{
//    const int COUNT = int(arg);
//    int c = 0;
//    for(int j = 0;j < COUNT;++j)
//    for(__Type s;;){
//        if(c >= COUNT)
//            exit(0);
//#ifdef WWH
//        if(que.empty()){
//            sleep(0);
//        }else{
//            s = que.front();
//            que.pop();
//#else
//        if(!que.Pop(s)){
//            sleep(0);
//        }else{
//#endif
//            ++c;
//            if(!(c & 1023))
//                cout<<s<<endl;
//        }
//    }
//    return 0;
//}

#endif

int main(int argc,const char ** argv)
{
#if __DZ_TEST
    INIT_LOGGER(0);
    CSemaphore sem(-1);
    cout<<SEM_VALUE_MAX<<endl;
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
