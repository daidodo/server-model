#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <common/Logger.h>
#include <common/DataStream.h>
#include <common/Configuration.h>
#include <common/Sockets.h>

using namespace std;

#include "TestCmd.h"
#include "comm.h"

CConfiguration config;
CSockAddr servAddr,clientAddr;
CTcpConnSocket tcpSock;
CUdpSocket udpSock;

bool initTcp()
{
    if(!tcpSock.IsValid() && !tcpSock.Connect(servAddr)){
        cerr<<"connect to server servAddr="<<servAddr.ToString()<<", "
            <<CSocket::ErrMsg()<<"\n";
        return false;
    }
    return true;
}

bool initUdp()
{
    if(!udpSock.IsValid()
        && !(udpSock.Socket(servAddr) && udpSock.Bind(clientAddr)))
    {
        cerr<<"init udp socket error for servAddr="<<servAddr.ToString()
            <<", clientAddr="<<clientAddr.ToString()<<", "<<CSocket::ErrMsg()<<"\n";
        return false;
    }
    return true;
}

bool test_entry(int choice)
{
    switch(choice){
        case 1:{     //Emule Query
            if(initTcp())
                return Query(tcpSock.FD(),clientAddr.GetPort());
            break;}
        case 2:     //Batch Emule Query
            return BatchQuery(servAddr);
        default:
            cerr<<"unknown test choice="<<choice<<endl;
    }
    return false;
}

int main(int argc,char ** argv)
{
    __DZ_STRING configfile;
    if(argc == 1)
        configfile = "test.conf";
    else
        configfile = argv[1];
    config.Load(configfile.c_str());
    __DZ_STRING ip = config.GetString("server.ip");
    U16 port = config.GetInt("server.port",9530);
    if(!servAddr.SetAddr(ip,port)){
        cout<<"server ip="<<ip<<",port="<<port<<" error\n";
        return 1;
    }
    int choice;
    if(argc < 3){
        cout<<"input choice:";
        while(cin>>choice){
            if(!test_entry(choice))
                sleep(2);
            cout<<"\ninput choice:";
        }
    }else{
        choice = atoi(argv[2]);
        if(!test_entry(choice))
            sleep(2);
    }
    return 0;
}
