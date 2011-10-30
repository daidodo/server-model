#include "comm.h"

using namespace std;

bool SendRecvPack(int fd,__DZ_VECTOR(char) & data,bool recv,bool udp)
{
    if(send(fd,&data[0],data.size(),0) < 0){
        cout<<"send error"<<CSocket::ErrMsg()<<"\n";
        data.clear();
        return false;
    }
    cout<<"send data finished\n";
    if(recv)    //有回应信息
        return udp ? RecvUdpPack(fd,data) : RecvPack(fd,data);
    return true;
}

bool RecvPack(int fd,__DZ_VECTOR(char) & data)
{
    const char HTTP_SIGN[] = {'H','T','T','P'};
    const char HTTP_END[] = {'\r','\n','\r','\n'};
    data.resize(HEAD_LEN);
    if(recv(fd,&data[0],HEAD_LEN,MSG_WAITALL) < 0){
        cout<<"recv package head error"<<CSocket::ErrMsg()<<"\n";
        data.clear();
        return false;
    }
    bool useHttp = !memcmp(&data[0],HTTP_SIGN,sizeof HTTP_SIGN);
    if(useHttp)
        cout<<"HTTP HEAD\n";
    for(size_t i = 0;useHttp;){
        ssize_t n = Tools::StringMatch(&data[i],data.size() - i,HTTP_END,sizeof HTTP_END);
        if(n >= 0){
            data.erase(data.begin(),data.begin() + i + n + sizeof HTTP_END);
            assert(data.size() < HEAD_LEN);
            i = data.size();
            size_t left = HEAD_LEN - i;
            data.resize(i + left);
            if(recv(fd,&data[i],left,MSG_WAITALL) < 0){
                cout<<"recv cmd head error"<<CSocket::ErrMsg()<<"\n";
                data.clear();
                return false;
            }
            useHttp = false;
        }else{
            i = data.size();
            if(i > sizeof HTTP_END)
                i -= sizeof HTTP_END;
            data.resize(i + HEAD_LEN);
            if(recv(fd,&data[i],HEAD_LEN,MSG_WAITALL) < 0){
                cout<<"recv http error"<<CSocket::ErrMsg()<<"\n";
                data.clear();
                return false;
            }
        }
    }
    U32 body_len;
    {
        CInByteStream ds(&data[LEN_OFFSET],data.size() - LEN_OFFSET);
        if(!(ds>>body_len)){
            cerr<<"decode length error\n";
            data.clear();
            return false;
        }
    }
    data.resize(body_len + HEAD_LEN);
    if(recv(fd,&data[HEAD_LEN],body_len,MSG_WAITALL) < 0){
        cout<<"recv body_len="<<body_len<<" error"<<CSocket::ErrMsg()<<"\n";
        data.clear();
        return false;
    }
#if(USE_ENCRYPT)
    __DZ_VECTOR(char) decryptData;
    CEncryptorAes aes;
    aes.SetKey(&data[0],ENCRYPT_KEY_LEN);
    int n = aes.Decrypt(data,ENCRYPT_FROM,decryptData);
    if(n < 0){
        cerr<<"decrypt cmd data="<<Tools::DumpHex(data)<<" return "<<n<<endl;
        return 0;
    }
    cout<<"decryptData="<<Tools::DumpHex(decryptData)<<endl;
    data.swap(decryptData);
#endif
    return true;
}

bool RecvUdpPack(int fd,__DZ_VECTOR(char) & data)
{
    const size_t MAX_LEN = 1024;
    data.resize(MAX_LEN);
    ssize_t n = recv(fd,&data[0],HEAD_LEN,MSG_WAITALL);
    if(n <= 0){
        cout<<"recv package head error"<<CSocket::ErrMsg()<<"\n";
        data.clear();
        return false;
    }
    data.resize(n);
    return true;
}

bool DecodeHead(CInByteStream & ds)
{
    U16 ver;
    __CmdType cmd_type;
    U32 seq,len;
    ds>>ver>>cmd_type>>seq>>len;
    cout<<"ver = "<<ver<<endl
        <<"cmd_type = "<<U32(cmd_type)<<endl
        <<"seq = "<<seq<<endl
        <<"length = "<<len<<endl
        ;
    return ds;
}

bool DecodeUdpHead(CInByteStream & ds)
{
    U16 ver;
    __CmdType cmd_type;
    U32 seq;
    ds>>ver>>cmd_type>>seq;
    cout<<"ver = "<<ver<<endl
        <<"cmd_type = "<<U32(cmd_type)<<endl
        <<"seq = "<<seq<<endl
        ;
    return ds;
}
