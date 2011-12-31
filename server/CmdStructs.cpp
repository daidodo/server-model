#include <Logger.h>
#include "CmdStructs.h"

NS_SERVER_BEGIN

//struct CCmdStatsItems
std::string CCmdStatsItems::ToString() const
{
    std::ostringstream oss;
    size_t t = succ_ + fail_;
    oss<<"succ_="<<succ_
        <<", fail_="<<fail_
        <<", totalTimeUs_="<<totalTimeUs_
        <<", aveTimeUs_="<<(t ? totalTimeUs_ / t : 0)
        <<", maxTimeUs_="<<maxTimeUs_
        <<", minTimeUs_="<<minTimeUs_
        ;
    return oss.str();
}

//class CCommandStats
__CmdStats * CCommandStats::stats_ = 0;

bool CCommandStats::timeStats_ = false;

CCommandStats::~CCommandStats(){
    typedef __CmdStats::guard_type  guard_type;
    typedef __CmdStats::mapped_type mapped_type;
    if(stats_){
        if(timeStats_)
            timeUs_ = Tools::GetTimeUs() - timeUs_;
        guard_type g(stats_->GetLock());
        mapped_type & item = (*stats_)[id_];
        succ_ ? ++item.succ_ : ++item.fail_;
        if(timeStats_){
            item.totalTimeUs_ += timeUs_;
            if(item.maxTimeUs_ < timeUs_)
                item.maxTimeUs_ = timeUs_;
            if(item.minTimeUs_ < 0 || timeUs_ < item.minTimeUs_)
                item.minTimeUs_ = timeUs_;
        }
    }
}

//business

NS_SERVER_END
