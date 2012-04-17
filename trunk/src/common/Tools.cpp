#include <sys/time.h>
#include <sys/resource.h>   //struct rlimit,getrlimit,RLIMIT_NOFILE,RLIM_INFINITY,setrlimit
#include <sys/types.h>
#include <sys/sysctl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/sysinfo.h>    //sysinfo
#include <arpa/inet.h>      //struct in_addr,htonl,inet_ntop,AF_INET,ntohl
#include <signal.h>
#include <unistd.h>
#include <cxxabi.h>         //abi::__cxa_demangle
#include <sstream>          //std::ostringstream
#include <cctype>           //std::isspace
#include <algorithm>        //std::min
#include <cstring>
#include <cassert>
#include <iomanip>          //std::setw
#include "Tools.h"

NS_SERVER_BEGIN

namespace Tools{

    std::string DumpHex(const char * v, size_t sz,char sep,bool hasLen)
    {
        const char DIGIT[] = "0123456789ABCDEF";
        assert(v);
        std::string ret;
        if(hasLen){
            std::ostringstream oss;
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

    std::string DumpStr(const char * v,size_t sz,char replace,bool hasLen)
    {
        const char DEFAULT = '.';
        const char TRAN_CHAR = '\\';
        const char FOLLOW_CHAR[] = "abtnvfr";
        assert(v);
        if(!IsReadable(replace))
            replace = DEFAULT;
        std::string ret;
        if(hasLen){
            std::ostringstream oss;
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

    std::string DumpVal(const char * v,size_t sz,int base,bool hasLen)
    {
        const char TRAN_CHAR = '\\';
        const char FOLLOW_CHAR[] = "abtnvfr";
        const char DIGIT[] = "0123456789ABCDEF";
        assert(v);
        std::string ret;
        if(hasLen){
            std::ostringstream oss;
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
    std::string Dump(const char * v,size_t sz,size_t show_sz,bool hasLen)
    {
        const size_t PRE_READ = 32;
        std::string ret;
        if(hasLen){
            std::ostringstream oss;
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

    std::string DumpFormat(const char * v, size_t sz)
    {
        if(!v || !sz)
            return "";
        const size_t LINE_WIDTH = 4;
        const size_t CHARS_PER_LINE = 16;
        size_t lines = (sz + CHARS_PER_LINE - 1) / CHARS_PER_LINE;
        size_t lw = 0;
        for(;lines > 0;lines >>= 8, lw += 2);
        if(lw < LINE_WIDTH)
            lw = LINE_WIDTH;
        std::ostringstream oss;
        oss.fill('0');
        oss<<std::hex;
        for(size_t ln = 0;ln < sz;ln += CHARS_PER_LINE){
            oss<<std::setw(lw)<<ln<<"h: ";
            const size_t left = std::min(CHARS_PER_LINE, sz - ln);
            oss<<DumpHex(v + ln,left,' ',false);
            for(size_t i = left;i < CHARS_PER_LINE;++i)
                oss<<"   ";
            oss<<"; ";
            for(size_t i = 0;i < left;++i)
                oss<<((v[ln + i] > 31 && v[ln + i] < 127) ? v[ln + i] : '.');
            oss<<std::endl;
        }
        return oss.str();
    }

    std::string UnHex(const char * v,size_t sz)
    {
        typedef const char * __Ptr;
        assert(v);
        std::string ret;
        ret.reserve(sz >> 1);
        int r = -1;
        for(__Ptr i = v,e = v + sz;i < e;++i){
            int t = UnHexChar(*i);
            if(r < 0){
                if(t >= 0)
                    r = t;
            }else{
                if(t >= 0)
                    r = (r << 4) + t;
                ret.push_back(r);
                r = -1;
            }
        }
        if(r >= 0)
            ret.push_back(r);
        return ret;
    }


    std::string Trim(std::string str)
    {
        size_t i = 0;
        for(;i < str.length() && std::isspace(str[i]);++i);
        size_t j = str.length();
        for(;j > i + 1 && std::isspace(str[j - 1]);--j);
        return i < j ? str.substr(i,j - i) : "";
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
        std::vector<ssize_t> next(tarlen,-1);
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

    std::string TimeString(U32 timeS,std::string format)
    {
        const int SIZE = 255;
        time_t t = timeS;
        struct tm cur_tm;
        localtime_r(&t,&cur_tm);
        char buf[SIZE] = {0};
        strftime(buf,SIZE,format.c_str(),&cur_tm);
        return buf;
    }

    std::string TimeStringUs(U64 timeUs,std::string format)
    {
        std::ostringstream oss;
        oss<<" "<<(timeUs % 1000000)<<" us";
        return TimeString(U32(timeUs / 1000000),format) + oss.str();
    }

    std::string IPv4String(U32 ip,bool hostByteOrder)
    {
        struct in_addr in;
        in.s_addr = hostByteOrder ? htonl(ip) : ip;
        char buf[46];
        if(!inet_ntop(AF_INET,&in,buf,sizeof buf))
            return "ERROR_IP";
        return buf;
    }

    U32 IPv4FromStr(std::string ip,bool hostByteOrder)
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

    U64 GetPhysicalMemorySize()
    {
        struct sysinfo si;
        if(0 != sysinfo(&si))
            return 0;
        if(si.mem_unit > 0)
            return U64(si.totalram) * si.mem_unit;
        return si.totalram;
    }

    size_t GetPageSize()
    {
#ifdef PAGE_SIZE
        return PAGE_SIZE;
#else
        return (size_t)sysconf(_SC_PAGESIZE);
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

    std::string ToStringBits(U32 v, const char * const * name, size_t name_len)
    {
        std::ostringstream oss;
        oss<<v;
        if(name && name_len){
            bool empty = true;
            for(size_t i = 0;i < name_len;++i){
                if(0 == (v & (1 << i)))
                    continue;
                if(empty){
                    oss<<"(";
                    empty = false;
                }else
                    oss<<" | ";
                if(name[i])
                    oss<<name[i];
                else
                    oss<<std::hex<<"0x"<<(1 << i);
            }
            if(!empty)
                oss<<")";
        }
        return oss.str();
    }

    std::string ErrorMsg(int error_no)
    {
        std::ostringstream os;
        os<<" errno="<<error_no<<" - ";
#if defined __USE_XOPEN2K || defined __USE_MISC
        const int MAX_BUF = 256;
        char buf[MAX_BUF];
        os<<strerror_r(error_no,buf,MAX_BUF);
#else
        os<<strerror(error_no);
#endif
        return os.str();
    }

    void Daemon(bool closeFiles)
    {
        // shield some signals
        signal(SIGALRM, SIG_IGN);
        signal(SIGINT,  SIG_IGN);
        signal(SIGHUP,  SIG_IGN);
        signal(SIGQUIT, SIG_IGN);
        signal(SIGPIPE, SIG_IGN);
        signal(SIGTTOU, SIG_IGN);
        signal(SIGTTIN, SIG_IGN);
        signal(SIGCHLD, SIG_IGN);
        signal(SIGTERM, SIG_IGN);
        // fork child process
        if(fork())
            exit(0);
        // creates  a new session
        if(setsid() == -1)
            exit(1);
        //do NOT close files
        if(closeFiles){
            for(int i = 3;i < NOFILE;++i)
                close(i);
        }
        if(chdir("/") < 0)
            exit(1);
        umask(0);
    }

    std::string GetHost(const std::string & url)
    {
        size_t from = url.find("//");
        if(std::string::npos == from)
            return "";
        from += 2;
        size_t to = url.find('/', from);
        if(std::string::npos == to)
            to = url.size();
        to -= from;
        if(!to)
            return "";
        return url.substr(from, to);
    }

    std::string UrlEncode(const std::string & url)
    {
        const char DIGIT[] = "0123456789ABCDEF";
        std::ostringstream oss;
        for(std::string::const_iterator i = url.begin();i != url.end();++i){
            if(isalnum(*i) ||
                    *i == '-' ||
                    *i == '_' ||
                    *i == '.' ||
                    *i == '~')
                oss<<*i;
            else if(*i == ' ')
                oss<<'+';
            else
                oss<<'%'<<DIGIT[*i >> 4]<<DIGIT[*i & 0xF];
        }
        return oss.str();
    }

    std::string UrlDecode(const std::string & url)
    {
        std::ostringstream oss;
        for(std::string::const_iterator i = url.begin();i != url.end();++i){
            if(*i == '%'){
                if(i + 2 < url.end()){
                    oss<<char((UnHexChar(*(i + 1)) << 4) + UnHexChar(*(i + 2)));
                    i += 2;
                }//else format error
            }else if(*i == '+')
                oss<<' ';
            else
                oss<<*i;
        }
        return oss.str();
    }

    std::string XmlEncode(const std::string & val)
    {
        std::ostringstream oss;
        for(std::string::const_iterator i = val.begin();i != val.end();++i){
            switch(*i){
                case '<':oss<<"&lt;";break;
                case '>':oss<<"&gt;";break;
                case '&':oss<<"&amp;";break;
                case '\'':oss<<"&apos;";break;
                case '"':oss<<"&quot;";break;
                default:oss<<*i;
            }
        }
        return oss.str();
    }

    std::string AbsFilename(const std::string & fname)
    {
        if(fname.empty())
            return fname;
        std::string cwd;
        if(fname[0] != '/'){
            cwd.resize(4096);
            if(NULL == getcwd(&cwd[0], cwd.size()))
                return "";
            size_t len = cwd.find_first_of('\0');
            if(!len)
                return "";
            if(std::string::npos != len)
                cwd.resize(len);
            if(*cwd.rbegin() != '/')
                cwd.push_back('/');
        }
        return (fname[0] != '/' ? cwd += fname : fname);
    }

    bool IsTimeout(U32 oldTime, U32 curtime, int timeout, int jumping)
    {
        if(!oldTime || timeout <= 0)
            return false;   //不会超时
        return curtime + jumping < oldTime
            || curtime > oldTime + timeout + jumping;
    }

    std::string CxxDemangle(const char * name)
    {
        assert(name);
        size_t len = 0;
        int status = 0;
        char * output = abi::__cxa_demangle(name, NULL, &len, &status);
        std::string ret;
        if(0 == status && output && len > 0)
            ret = output;
        else
            ret = name;
        if(output)
            free(output);
        return ret;
    }

    bool IsPrime(int v)
    {
        if(v < 2)
            return false;
        for(int i = 2;i * i <= v;++i)
            if((v % i) == 0)
                return false;
        return true;
    }

    int PrimeLess(int v)
    {
        for(;v >= 2;--v)
            if(IsPrime(v))
                return v;
        return 0;
    }

}//namespace Toos

NS_SERVER_END
