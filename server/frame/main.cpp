//for test
#define __DZ_TEST   0

#include "main.h"

#if __DZ_TEST
//#   include <string>
//#   include <ctime>
//#   include <Logger.h>
//#   include <Threads.h>
//#   include <Tools.h>
#   include <DataStream.h>
//#   include <LockInt.h>
//#   include <LockQueue.h>
//#   include <RingBuffer.h>
//#   include <Epoll.h>
//#   include <Poll.h>
//#   include <FdMap.h>
//#   include <Mutex.h>
//#   include <server/frame/CmdSock.h>
//#   include <List.h>
//#   include <Tree.h>
//#   include <Semaphore.h>

static void test1()
{
    std::string buf("����һ�����ĵķ������÷ְ����ǰ���˵�����¸�����˵��");

    CInBitStream bs(buf);
    bs.SetSource(buf);
    char a = 0;
    if(bs>>a)
        std::cout<<int(a)<<std::endl;
    std::cout<<bs.ToString()<<std::endl;

    short i = 0,b = 1;
    while(bs>>Manip::bits(b,i)){
        std::cout<<b<<"\t"<<i<<std::endl;
        std::cout<<bs.ToString()<<std::endl;
        b += 7;
    }
    std::cout<<b<<std::endl;
}

static void test2()
{
    COutByteStream ds(0);
    ds<<3;
    int a = 'a';
    COutBitStream bs(0);

    for(int i = 0;i < 5;++i){
        bs<<Manip::bits(i,a);
        std::cout<<i<<std::endl
            <<bs.ToString()<<std::endl;
    }
}

#endif

int main(int argc,const char ** argv)
{
#if __DZ_TEST
    INIT_LOGGER(0);
    test2();
#else
    //Ĭ�Ϸ����������ļ�
    const char * serverconf = DEFAULT_CONF_FILE;
    //Ĭ����־�����ļ�
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
            std::cout<<versionInfo()<<std::endl;
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
