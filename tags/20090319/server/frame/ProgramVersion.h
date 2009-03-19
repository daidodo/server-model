#ifndef DOZERG_PROGRAM_VERSION_20080126
#define DOZERG_PROGRAM_VERSION_20080126

/*
����汾��,CProgramVersion::High()Ϊ����,CProgramVersion::Low()Ϊʱ��
���ú�PROGRAM_VERSION_HIGH��PROGRAM_VERSION_LOW�õ�make��ʱ��
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
