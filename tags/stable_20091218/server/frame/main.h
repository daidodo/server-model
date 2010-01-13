#ifndef DOZERG_MAIN_H_20080514
#define DOZERG_MAIN_H_20080514

#include <iostream>
#include <signal.h>
#include <common/Tools.h>
#include <common/Logger.h>
#include <server/frame/ProgramVersion.h>
#include <server/MainServer.h>

using namespace std;
using namespace NS_SERVER;

//NS_SERVER_BEGIN
//
//#if defined(LOGGER) && !defined(LOGSYS)
//GLOBAL_LOGGER(logger,"");
//#endif
//
//NS_SERVER_END

static const char * programName;

static CMainServer * mainServer;

// 信号定义
static const int _SIG_SHOW_CONFIG = SIGRTMIN + 13; //显示配置信息

static const int _SIG_RECONFIG = SIGRTMIN + 14;    //重读配置文件

static const int _SIG_EXIT = SIGRTMIN + 15;        //正常退出

static const char * const DEFAULT_CONF_FILE = "../conf/server.conf";    //默认服务器配置文件

static const char * const DEFAULT_LOG_CONF_FILE = "../conf/logger.conf";//默认日志配置文件

static void printUsage()
{
    cout<<"Usage: "<<programName<<" [option]\n"
        <<"  options:\n"
        <<"                 run program\n"
        <<"  ?,-h,--help    print help message\n"
        <<"  -v             print program version\n"
        <<"  -c=FILENAME    specify config file, default '"<<DEFAULT_CONF_FILE<<"'\n"
        <<"  -l=FILENAME    specify log config file, default '"<<DEFAULT_LOG_CONF_FILE<<"'\n"
        <<"\n  To show the server config info, use:\n"
        <<"     kill -"<<_SIG_SHOW_CONFIG<<" (process id)\n"
        <<"  To reconfig the server, use:\n"
        <<"     kill -"<<_SIG_RECONFIG<<" (process id)\n"
        <<"  To exit normally, use:\n"
        <<"     kill -"<<_SIG_EXIT<<" (process id)\n"
        <<endl;
}

static __DZ_STRING versionInfo()
{
    __DZ_OSTRINGSTREAM oss;
    oss<<programName<<" version "<<CProgramVersion::High()
        <<"."<<CProgramVersion::Low();
#ifdef NDEBUG
    oss<<" release";
#else
    oss<<" debug";
#endif
    return oss.str();
}

static void reconfig(int)
{
    cout<<"\nbegin re-config server...\n";
    assert(mainServer);
    mainServer->Reconfig();
    cout<<"\nre-config finished\n";
}

static void showConfig(int)
{
    assert(mainServer);
    mainServer->ShowConfig(versionInfo());
}

static void exitNormally(int)
{
    exit(0);
}

static void registerSignal()
{
    if(signal(_SIG_RECONFIG,reconfig) == SIG_ERR)
        cerr<<"register RECONFIG signal failed"<<endl;
    if(signal(_SIG_SHOW_CONFIG,showConfig) == SIG_ERR)
        cerr<<"register SHOW_CONFIG signal failed"<<endl;
    if(signal(_SIG_EXIT,exitNormally) == SIG_ERR)
        cerr<<"register EXIT signal failed"<<endl;
}

#endif
