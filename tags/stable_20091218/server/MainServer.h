#ifndef DOZERG_MAIN_SERVER_H_20071219
#define DOZERG_MAIN_SERVER_H_20071219

#include <fstream>
#include <list>
#include <common/Configuration.h>
#include <common/SharedPtr.h>
#include <common/LockQueue.h>
#include <common/LockInt.h>
#include <common/FdMap.h>
#include <server/frame/CmdSock.h>

NS_SERVER_BEGIN

class CAcceptServer;
class CEpollServer;
class CPollServer;
class CTcpServer;
class CCmdHandler;
class CStatsServer;

struct CMainServer
{
//typedefs:
    typedef CConfiguration                  __Config;
    typedef CCmdSock                        __CmdSock;
    typedef CFdEvent                        __FdEvent;
    typedef CSharedPtr<__CmdSock>           __SockPtr;
    typedef CLockQueue<int>                 __FdQue;
    typedef CLockQueue<__FdEvent>           __FdEventQue;
    typedef CFdSockMap<__CmdSock,__SockPtr> __FdSockMap;
    typedef Tools::CTriple<
        QCmdBase *,int,__SockPtr>           __CmdTriple;
    typedef CLockQueue<__CmdTriple>         __QueryCmdQue;
//functions:
    CMainServer();
    ~CMainServer();
    void Init(const char * serverconf);
    void Reconfig();            //重新读取配置文件
    void ShowConfig(__DZ_STRING verInfo) const;    //显示当前配置信息
	void StartServer();
//members:
    //threads
    CAcceptServer * acceptServer_;
    CEpollServer *  epollServer_;
    CPollServer *   pollServer_;
    CTcpServer *    tcpServer_;
    CCmdHandler *   cmdHandler_;
    CStatsServer *  statsServer_;
    //objects
    __FdSockMap     fdSockMap_;
    __FdEventQue    addingFdQue_;
    __FdQue         removeFdQue_;
    __FdEventQue *  eventFdQue_;
    __QueryCmdQue   queryCmdQue_;
    //configs
    __DZ_STRING configFile_;            //读取服务器配置的文件名
    __DZ_STRING showConfigFile_;        //显示服务器配置的文件名,不能重新配置
    bool        useEpoll_;              //使用epoll还是poll,不能重新配置
    bool        serverStatsOn_;         //是否开启统计线程,不能重新配置
    int         acceptServerStatckSz_;  //各个线程的栈大小,不能重新配置
    int         pollServerStackSz_;         
    int         epollServerStackSz_;
    int         tcpServerStackSz_;
    int         cmdHandlerStackSz_;
    int         statsServerStackSz_;
    int         tcpServerThreadCount_;  //线程数量,不能重新配置
    //business

};

NS_SERVER_END

#endif
