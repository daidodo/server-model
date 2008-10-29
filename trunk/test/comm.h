#ifndef COMM_H
#define COMM_H

#include <iostream>
#include <ctime>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <common/DataStream.h>
#include <common/Configuration.h>
#include <common/EncryptorAes.h>
#include <common/Sockets.h>

#define USE_ENCRYPT 0

const size_t HEAD_LEN = 12;
const size_t LEN_OFFSET = 8;
const size_t ENCRYPT_KEY_LEN = 8;
const size_t ENCRYPT_FROM = 12;

typedef U16 __CmdType;

inline COutByteStream & PackHead(COutByteStream & ds,U32 cmd_type)
{
    return ds<<U16(USE_ENCRYPT) //version
        <<__CmdType(cmd_type)   //cmdtype
        <<U32(99887766)         //seq
        <<U32(0)                //len
        ;
}

inline COutByteStream & PackUdpHead(COutByteStream & ds,U32 cmd_type)
{
    return ds<<U16(1)           //version
        <<__CmdType(cmd_type)   //cmdtype
        <<U32(99887766)         //seq
        ;
}

inline void SetLength(COutByteStream & ds)
{
    ds<<Manip::offset_value(LEN_OFFSET,ds.Size() - HEAD_LEN);
}

//recv表示是否有回应数据
bool SendRecvPack(int fd,__DZ_VECTOR(char) & data,bool recv = false,bool udp = false);

bool RecvPack(int fd,__DZ_VECTOR(char) & data);

bool RecvUdpPack(int fd,__DZ_VECTOR(char) & data);

//解码包头
bool DecodeHead(CInByteStream & ds);

bool DecodeUdpHead(CInByteStream & ds);



#endif
