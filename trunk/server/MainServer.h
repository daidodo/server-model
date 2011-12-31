#ifndef DOZERG_MAIN_SERVER_H_20071219
#define DOZERG_MAIN_SERVER_H_20071219

#include <Configuration.h>
#include <SharedPtr.h>
#include <LockQueue.h>
#include <FdMap.h>
#include "frame/CmdSock.h"

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
    void Reconfig();            //���¶�ȡ�����ļ�
    void ShowConfig(std::string verInfo) const;    //��ʾ��ǰ������Ϣ
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
    std::string configFile_;            //��ȡ���������õ��ļ���
    std::string showConfigFile_;        //��ʾ���������õ��ļ���,������������
    bool        useEpoll_;              //ʹ��epoll����poll,������������
    bool        serverStatsOn_;         //�Ƿ���ͳ���߳�,������������
    int         acceptServerStatckSz_;  //�����̵߳�ջ��С,������������
    int         pollServerStackSz_;         
    int         epollServerStackSz_;
    int         tcpServerStackSz_;
    int         cmdHandlerStackSz_;
    int         statsServerStackSz_;
    int         tcpServerThreadCount_;  //�߳�����,������������
    //business

};

NS_SERVER_END

#endif
