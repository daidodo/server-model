#include <sstream>
#include <ctime>
#include <common/Threads.h>
#include <common/Configuration.h>
#include "comm.h"
#include "TestCmd.h"

using namespace std;

extern CConfiguration config;

static bool autoSendQuery(int fd,U16 port,U32 cmd_id,__DZ_STRING fileHash,__DZ_STRING clientHash,bool bConfig = true)
{
    bool useHttp = false;
    __DZ_STRING peerId = "1234";
    if(bConfig){
        useHttp = config.GetInt("cmd.use.http.1");
        peerId = config.GetString("cmd.body.peer.id.1");
    }
    cout<<"QUERY:\n"
        <<"fileHash = "<<Tools::DumpHex(fileHash)<<endl
        <<"clientHash = "<<Tools::DumpHex(clientHash)<<endl
        <<"peerId = "<<peerId<<endl
        ;
    __DZ_VECTOR(char) data;{
        COutByteStream ds;
        __DZ_VECTOR(char) tmp;
        //encrypt
#if(USE_ENCRYPT)
        COutByteStream dss;
        PackHead(dss,cmd_id)<<fileHash<<clientHash<<peerId;
        dss.ExportData(tmp);
        CEncryptorAes aes;
        aes.SetKey(&tmp[0],ENCRYPT_KEY_LEN);
        int n = aes.Encrypt(tmp,ENCRYPT_FROM,data);
        if(n < 0){
            cerr<<"encrypt data="<<Tools::DumpHex(tmp)<<" return "<<n<<endl;
            return false;
        }
        ds<<Manip::raw(&data[0],data.size());
#else
        PackHead(ds,cmd_id)<<fileHash<<clientHash<<peerId;
#endif
        SetLength(ds);
        ds.ExportData(tmp);
        //http
        if(useHttp){
            __DZ_OSTRINGSTREAM oss;
            oss<<"POST http://127.0.0.1:12345/ HTTP/1.1\r\nContent-Length: "<<tmp.size()
                <<"\r\nContent-Type: application/octet-stream\r\nConnection: Close\r\n\r\n";
            __DZ_STRING http = oss.str();
            data.assign(http.begin(),http.end());
            data.insert(data.end(),tmp.begin(),tmp.end());
        }else
            data.swap(tmp);
    }
    if(!SendRecvPack(fd,data,true))
        return false;
    //½âÂë½á¹û
    cout<<"RESP:\n";
    CInByteStream ds(data);
    if(!DecodeHead(ds)){
        cout<<"decode head error\ndata="<<Tools::DumpHex(data)<<endl;
        return false;
    }
    U8 res;
    if(!(ds>>res>>fileHash>>clientHash>>peerId)){
        cerr<<"decode body error\ndata="<<Tools::DumpHex(data)<<endl;
        return false;
    }
    cout<<"Result = "<<S32(res)<<endl
        <<"fileHash = "<<Tools::DumpHex(fileHash)<<endl
        <<"clientHash = "<<Tools::DumpHex(clientHash)<<endl
        <<"peerId = "<<peerId<<endl
        <<endl;
    return true;
}


bool Query(int fd,U16 port)
{
    U32 cmd_id = config.GetInt("cmd.ID.1");
    __DZ_STRING fileHash = config.GetString("cmd.body.file.hash.1");
    __DZ_STRING clientHash = config.GetString("cmd.body.client.hash.1");
    return autoSendQuery(fd,port,cmd_id,fileHash,clientHash);
}

struct ThreadArg
{
    int cmd_count_;
    bool repeat_;
    const CSockAddr & servAddr;
    explicit ThreadArg(const CSockAddr & addr):servAddr(addr){}
};

const char * randStr(char * str,size_t sz)
{
    for(size_t i = 0;i < sz;++i)
        str[i] = 'a' + rand() % ('z' - 'a');
    return str;
}

void * thread_proc(void * arg)
{
    assert(arg);
    const ThreadArg & targ = *(ThreadArg *)arg;
    __DZ_VECTOR(char) data;
    const U32 cmd_id = 161;
    do{
        CTcpConnSocket conn;
        if(!conn.Connect(targ.servAddr)){
            cerr<<"connect to servAddr="<<targ.servAddr.ToString()<<" error\n";
            sleep(1);
            continue;
        }
        char str[20] = {0};
        for(int i = 0;i < targ.cmd_count_;++i){
            __DZ_STRING fileHash = randStr(str,20);
            __DZ_STRING clientHash = randStr(str,20);
            if(!autoSendQuery(conn.FD(),0,cmd_id,fileHash,clientHash,false))
                sleep(1);
        }
    }while(targ.repeat_);
    return 0;
}

bool BatchQuery(const CSockAddr & servAddr)
{
    ThreadArg arg(servAddr);
    int thread_count = config.GetInt("batch.test.thread.count.2",1,1);
    arg.cmd_count_ = config.GetInt("batch.test.cmd.per.thread.2",1,1);
    arg.repeat_ = config.GetInt("batch.test.repeat.2");
    srand(U32(time(0)));
    CThreads thread(thread_proc,thread_count);
    thread.StartThreads("Batch Query",&arg);
    thread.WaitAll();
    return true;
}
