#ifndef DOZERG_COMMAND_H_20071219
#define DOZERG_COMMAND_H_20071219

#include <deque>
#include <server/CommandBase.h>
#include <server/CmdStructs.h>

NS_SERVER_BEGIN

//command ID definition
const U32 CMD_QUERY = 161;
const U32 CMD_RESP  = 162;

//命令结构定义

//Query
struct CQueryCmd : public QCmdBase
{
    __DZ_STRING     fileHash_;
    __DZ_STRING     clientHash_;
    __DZ_STRING     peerId_;
    //functions:
    __DZ_STRING ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
};

//Response
struct CQueryRespCmd : public RCmdBase
{
    U8          result_;
    __DZ_STRING fileHash_;
    __DZ_STRING clientHash_;
    __DZ_VECTOR(__DZ_STRING) peerId_;
    //functions:
    explicit CQueryRespCmd(const QCmdBase & head);
    __DZ_STRING ToStringHelp() const;
    void EncodeParam(COutByteStream & ds) const;
};

NS_SERVER_END

#endif
