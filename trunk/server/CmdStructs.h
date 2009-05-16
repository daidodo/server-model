#ifndef DOZERG_STRUCT_DEFINE_H_20071220
#define DOZERG_STRUCT_DEFINE_H_20071220

/*
    Э���������õ��Ľṹ����
    ���ֶε���ϸ������ο�Э���ĵ�
//*/

#include <sstream>
#include <string>
#include <vector>
#include <common/Tools.h>
#include <common/DataStream.h>
#include <common/LockMap.h>
#include <common/impl/Alloc.h>

NS_SERVER_BEGIN

//����Command������ͳ�Ƶ���Ͷ���
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
