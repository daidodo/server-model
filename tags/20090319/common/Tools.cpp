#include <sstream>          //std::ostringstream
#include <cctype>           //std::isspace
#include <algorithm>        //std::min
#include <cassert>
#include <arpa/inet.h>      //struct in_addr,htonl,inet_ntop,AF_INET,ntohl
#include <sys/time.h>
#include <sys/resource.h>   //struct rlimit,getrlimit,RLIMIT_NOFILE,RLIM_INFINITY,setrlimit
#include <sys/types.h>
#include <sys/sysctl.h>
#include <endian.h>         //BYTE_ORDER,LITTLE_ENDIAN
#include "Tools.h"

NS_SERVER_BEGIN

namespace Tools{

    __DZ_STRING DumpHex(const char * v,size_t sz,char sep,bool hasLen)
    {
        const char DIGIT[] = "0123456789ABCDEF";
        assert(v);
        __DZ_STRING ret;
        if(hasLen){
            __DZ_OSTRINGSTREAM oss;
            oss<<"("<<sz<<")";
            ret = oss.str();
        }
        ret.reserve(ret.size() + (2 + (sep ? 1 : 0)) * sz);
        for(size_t i = 0;i < sz;++i){
            ret.push_back(DIGIT[(v[i] >> 4) & 0xF]);
            ret.push_back(DIGIT[v[i] & 0xF]);
            if(sep)
                ret.push_back(sep);
        }
        return ret;
    }

    __DZ_STRING DumpStr(const char * v,size_t sz,char replace,bool hasLen)
    {
        const char DEFAULT = '.';
        const char TRAN_CHAR = '\\';
        const char FOLLOW_CHAR[] = "abtnvfr";
        assert(v);
        if(!IsReadable(replace))
            replace = DEFAULT;
        __DZ_STRING ret;
        if(hasLen){
            __DZ_OSTRINGSTREAM oss;
            oss<<"("<<sz<<")";
            ret = oss.str();
        }
        ret.reserve(ret.size() + sz + (sz >> 2));
        for(;sz > 0;--sz,++v){
            if(*v == TRAN_CHAR){
                ret.push_back(TRAN_CHAR);
                ret.push_back(TRAN_CHAR);
            }else if(*v >= '\a' && *v <= '\r'){
                ret.push_back(TRAN_CHAR);
                ret.push_back(FOLLOW_CHAR[*v - '\a']);
            }else if(!*v){
                ret.push_back(TRAN_CHAR);
                ret.push_back('0');
            }else
                ret.push_back(IsReadable(*v) ? *v : replace);
        }
        return ret;
    }

    __DZ_STRING DumpVal(const char * v,size_t sz,int base,bool hasLen)
    {
        const char TRAN_CHAR = '\\';
        const char FOLLOW_CHAR[] = "abtnvfr";
        const char DIGIT[] = "0123456789ABCDEF";
        assert(v);
        __DZ_STRING ret;
        if(hasLen){
            __DZ_OSTRINGSTREAM oss;
            oss<<"("<<sz<<")";
            ret = oss.str();
        }
        ret.reserve(ret.length() + 2 * sz);
        for(;sz > 0;--sz,++v){
            if(*v == TRAN_CHAR){
                ret.push_back(TRAN_CHAR);
                ret.push_back(TRAN_CHAR);
            }else if(IsReadable(*v))
                ret.push_back(*v);
            else{
                ret.push_back(TRAN_CHAR);
                if(*v >= '\a' && *v <= '\r')
                    ret.push_back(FOLLOW_CHAR[*v - '\a']);
                else if(!*v)
                    ret.push_back('0');
                else{
                    switch(base){
                        case 16:{       //16进制
                            ret.push_back(DIGIT[(*v >> 4) & 0xF]);
                            ret.push_back(DIGIT[*v & 0xF]);
                            break;}
                        default:       //8进制
                            ret.push_back(DIGIT[(*v >> 6) & 3]);
                            ret.push_back(DIGIT[(*v >> 3) & 7]);
                            ret.push_back(DIGIT[*v & 7]);
                    }
                }
            }
        }
        return ret;
    }

    //预读取前PRE_READ个字符，统计可读字符个数，然后选择合适的转换函数
    __DZ_STRING Dump(const char * v,size_t sz,size_t show_sz,bool hasLen)
    {
        const size_t PRE_READ = 32;
        __DZ_STRING ret;
        if(hasLen){
            __DZ_OSTRINGSTREAM oss;
            oss<<"("<<sz<<")";
            ret = oss.str();
        }
        size_t readable = 0;
        size_t check_len = std::min(sz,PRE_READ);
        for(const char *t = v,*e = v + check_len;t < e;++t)
            if(*t && IsReadable(*t))
                ++readable;
        if(readable <= (check_len >> 1))
            ret += DumpHex(v,std::min(sz,show_sz),' ',false);
        else if(readable < check_len)
            ret += DumpVal(v,std::min(sz,show_sz),8,false);
        else
            ret += DumpStr(v,std::min(sz,show_sz),'.',false);
        if(show_sz < sz)
            ret += "...";
        return ret;
    }

    __DZ_STRING UnHex(const char * v,size_t sz)
    {
        assert(v && sz);
        __DZ_STRING ret;
        ret.reserve(sz >> 1);
        for(size_t i = 1;i < sz;i += 2){
            int t1 = UnHexChar(v[i - 1]);
            int t2 = UnHexChar(v[i]);
            if(t1 < 0 || t2 < 0)
                break;
            ret.push_back((t1 << 4) + t2);
        }
        return ret;
    }


    __DZ_STRING Trim(__DZ_STRING str)
    {
        size_t i = 0;
        for(;i < str.length() && std::isspace(str[i]);++i);
        size_t j = str.length() - 1;
        for(;j > i && std::isspace(str[j]);--j);
        return i <= j ? str.substr(i,j + 1 - i) : "";
    }

    size_t FindFirst(const char * str,size_t len,char ch,size_t off)
    {
        if(!str)
            return len;
        for(size_t i = off;i < len;++i)
            if(str[i] == ch)
                return i;
        return len;
    }

    ssize_t StringMatch(const char * src,size_t srclen,const char * target,size_t tarlen)
    {
        assert(src && srclen && target && tarlen);
        for(size_t i = 0,j = 0,k = i;i < srclen;){
            if(src[i++] == target[j]){
                if(!j)
                    k = i;
                if(++j >= tarlen)
                    return k - 1;
            }else if(j){
                j = 0;
                i = k;
            }
        }
        return -1;
    }

    //使用kmp算法的字符串匹配
    ssize_t StringMatchKMP(const char * src,size_t srclen,const char * target,size_t tarlen)
    {
        assert(src && tarlen && target && tarlen);
        //compute next array
        __DZ_VECTOR(ssize_t) next(tarlen,-1);
        size_t i = 0,j = tarlen - 1;
        for(ssize_t k = -1;i < j;){
            while(k != -1 && target[i] != target[k])
                k = next[k];
            ++i,++k;
            next[i] = (target[i] == target[k] ? next[k] : k);
        }
        //kmp match
        i = j = 0;
        while(i < tarlen && j < srclen){
            if(target[i] == src[j]){
                ++i,++j;
            }else if(next[i] != -1) 
                i = next[i];
            else{
                i = 0;
                ++j;
            }
        }
        return (i >= tarlen ? ssize_t(j - i) : -1);
    }

    void GetTimespec(U32 timeMs,struct timespec & ts)
    {
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS
        clock_gettime(CLOCK_REALTIME,&ts);
        ts.tv_sec += timeMs / 1000;
        ts.tv_nsec += (timeMs % 1000) * 1000000;
#else
        struct timeval now;
        gettimeofday(&now, NULL);
        timeMs += (now.tv_usec / 1000);
        ts.tv_sec = now.tv_sec + timeMs / 1000;
        ts.tv_nsec = (timeMs % 1000) * 1000000;
#endif
    }

    U64 GetTimeUs(U64 elapse)
    {
        U64 cur = 0;
#if defined(_POSIX_TIMERS) && _POSIX_TIMERS
        struct timespec ts;
        clock_gettime(CLOCK_REALTIME,&ts);
        cur = ts.tv_sec;
        cur *= 1000000;
        cur += ts.tv_nsec / 1000;
#else
        struct timeval now;
        gettimeofday(&now,0);
        cur = now.tv_sec;
        cur *= 1000000;
        cur += now.tv_usec;
#endif
        return cur + elapse;
    }

    __DZ_STRING TimeString(U32 timeS,__DZ_STRING format)
    {
        const int SIZE = 255;
        time_t t = timeS;
        struct tm cur_tm;
        localtime_r(&t,&cur_tm);
        char buf[SIZE] = {0};
        strftime(buf,SIZE,format.c_str(),&cur_tm);
        return buf;
    }

    __DZ_STRING TimeStringUs(U64 timeUs,__DZ_STRING format)
    {
        __DZ_OSTRINGSTREAM oss;
        oss<<" "<<(timeUs % 1000000)<<" us";
        return TimeString(U32(timeUs / 1000000),format) + oss.str();
    }

    __DZ_STRING IPv4String(U32 ip,bool hostByteOrder)
    {
        struct in_addr in;
        in.s_addr = hostByteOrder ? htonl(ip) : ip;
        char buf[46];
        if(!inet_ntop(AF_INET,&in,buf,sizeof buf))
            return "ERROR_IP";
        return buf;
    }

    U32 IPv4FromStr(__DZ_STRING ip,bool hostByteOrder)
    {
        struct in_addr in;
        if(inet_pton(AF_INET,ip.c_str(),&in) == 0)
            return 0;
        return hostByteOrder ? ntohl(in.s_addr) : in.s_addr;	
    }

    bool SetMaxFileDescriptor(U32 numfiles)
    {
        struct rlimit rt;
        rt.rlim_max = rt.rlim_cur = numfiles;
        return setrlimit(RLIMIT_NOFILE,&rt) == 0;
    }

    U32 GetMaxFileDescriptor()
    {
        U32 ret = 0;
        struct rlimit rt;
        if(!getrlimit(RLIMIT_NOFILE,&rt)&& rt.rlim_cur != RLIM_INFINITY)
            ret = rt.rlim_cur - 1;
        return ret;
    }

    int GetProcessorCount()
    {
#if defined(__APPLE__)
        int mib[2], ncpus;
        size_t len = sizeof(ncpus);
        mib[0] = CTL_HW;
        mib[1] = HW_NCPU;
        sysctl(mib,2,&ncpus,&len,NULL,0);
        return ncpus;
#elif defined(__linux__)
        return (int)sysconf(_SC_NPROCESSORS_ONLN);
#else
        return -1;
#endif
    }

    const char * ProgramName(const char * argstr)
    {
        const char * ret = argstr;
        for(const char * cur = argstr;cur && *cur;++cur)
            if(*cur == '/')
                ret = cur + 1;
        return ret;
    }

    bool ExtractArg(const char * argstr,const char * pattern,const char *& result)
    {
        if(!argstr || !pattern)
            return false;
        for(;*pattern;++pattern,++argstr)
            if(*pattern != *argstr)
                return false;
        result = *argstr ? argstr : 0;
        return true;
    }

    __DZ_STRING ErrorMsg(int error_no)
    {
        const int MAX_BUF = 256;
        char buf[MAX_BUF];
        __DZ_OSTRINGSTREAM os;
        os<<" errno="<<error_no<<" - "<<strerror_r(error_no,buf,MAX_BUF);
        return os.str();
    }


}//namespace Toos

NS_SERVER_END
