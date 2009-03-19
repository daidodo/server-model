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
typedef CLockMap<U32,Tools::CTriple<size_t,size_t,U64> > __CmdStats; //ID -> (succ,fail,timeUs)

class CCommandStats
{
    static __CmdStats * stats_;
    static bool         timeStats_;
    U64         timeUs_;
    const U32   id_;
    bool        succ_;
public:
    static void SetCmdStats(__CmdStats * s){stats_ = s;}
    static void TimeStats(bool s){timeStats_ = s;}
    static bool TimeStats(){return timeStats_;}
    explicit CCommandStats(int id,bool succ = false)
        : timeUs_(stats_ && timeStats_ ? Tools::GetTimeUs() : 0)
        , id_(id)
        , succ_(succ){}
    void Succ(bool succ = true){succ_ = succ;}
    ~CCommandStats(){
        typedef __CmdStats::guard_type guard_type;
        if(stats_){
            if(timeStats_)
                timeUs_ = Tools::GetTimeUs() - timeUs_;
            guard_type g(stats_->GetLock());
            succ_ ? ++(*stats_)[id_].first : ++(*stats_)[id_].second;
            (*stats_)[id_].third += timeUs_;
        }
    }
};

//business

NS_SERVER_END

#endif
