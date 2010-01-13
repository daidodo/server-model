#ifndef DOZERG_PROGRAM_VERSION_20080126
#define DOZERG_PROGRAM_VERSION_20080126

/*
程序版本号,CProgramVersion::High()为日期,CProgramVersion::Low()为时刻
利用宏PROGRAM_VERSION_HIGH和PROGRAM_VERSION_LOW得到make的时间
//*/

#include <common/impl/Config.h>

NS_SERVER_BEGIN

struct CProgramVersion
{
    static int High(){return pv_.high_;}
    static int Low(){return pv_.low_;}
private:
    static CProgramVersion pv_;
    int high_,low_;
    CProgramVersion(int h,int l):high_(h),low_(l){}
};

NS_SERVER_END

#endif
