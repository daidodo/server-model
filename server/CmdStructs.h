#ifndef DOZERG_STRUCT_DEFINE_H_20071220
#define DOZERG_STRUCT_DEFINE_H_20071220

/*
    协议命令中用到的结构定义
    各字段的详细意义请参考协议文档
//*/

#include <sstream>
#include <string>
#include <vector>
#include <common/Tools.h>
#include <common/DataStream.h>
#include <common/LockMap.h>
#include <common/impl/Alloc.h>

NS_SERVER_BEGIN

//用于Command处理结果统计的类和定义
struct CCmdStatsItems
{
    size_t  succ_;
    size_t  fail_;
    S64     totalTimeUs_;
    S64     minTimeUs_;
    S64     maxTimeUs_;
    CCmdStatsItems()
        : succ_(0)
        , fail_(0)
        , totalTimeUs_(0)
        , minTimeUs_(-1)
        , maxTimeUs_(0)
    {}
    __DZ_STRING ToString() const;
};

typedef CLockMap<U32,CCmdStatsItems> __CmdStats; //ID -> (succ,fail,timeUs)

class CCommandStats
{
    static __CmdStats * stats_;
    static bool         timeStats_;
    S64         timeUs_;
    const U32   id_;
    bool        succ_;
public:
    static void SetCmdStats(__CmdStats * s){stats_ = s;}
    static void TimeStats(bool s){timeStats_ = s;}
    static bool TimeStats(){return timeStats_;}
    explicit CCommandStats(int id,bool succ = false)
        : timeUs_(stats_ && timeStats_ ? Tools::GetTimeUs() : 0)
        , id_(id)
        , succ_(succ)
    {}
    ~CCommandStats();
    void Succ(bool succ = true){succ_ = succ;}

};

//business

NS_SERVER_END

#endif
