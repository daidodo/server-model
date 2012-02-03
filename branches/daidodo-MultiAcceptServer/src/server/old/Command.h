#ifndef DOZERG_COMMAND_H_20071219
#define DOZERG_COMMAND_H_20071219

#include "CommandBase.h"
#include "CmdStructs.h"

NS_SERVER_BEGIN

//command ID definition
const U32 CMD_QUERY = 161;
const U32 CMD_RESP  = 162;

//命令结构定义

//Query
struct CQueryCmd : public QCmdBase
{
    std::string     fileHash_;
    std::string     clientHash_;
    std::string     peerId_;
    //functions:
    std::string ToStringHelp() const;
    bool DecodeParam(CInByteStream & ds);
};

//Response
struct CQueryRespCmd : public RCmdBase
{
    U8          result_;
    std::string fileHash_;
    std::string clientHash_;
    std::vector<std::string> peerId_;
    //functions:
    explicit CQueryRespCmd(const QCmdBase & head);
    std::string ToStringHelp() const;
    void EncodeParam(COutByteStream & ds) const;
};

NS_SERVER_END

#endif
