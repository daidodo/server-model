#ifndef DOZERG_DATA_STREAM_IMPL_H_20081016
#define DOZERG_DATA_STREAM_IMPL_H_20081016

#include <arpa/inet.h>          //ntohl
#include <algorithm>
#include <sstream>
#include "../CharBuffer.h"
#include "../Tools.h"
#include "Config.h"

NS_IMPL_BEGIN

class CDataStreamBase
{
    typedef CDataStreamBase __Myt;
    typedef void (__Myt::*__SafeBool)();
protected:
    typedef U16 __Length;   // type of length field (string length, array length, etc.)
public:
    enum ESeekDir{
        Begin,
        End,
        Cur
    };
    static const bool DEF_NET_BYTEORDER = true;    //默认使用网络字节序(true)还是本地字节序(false)
    explicit CDataStreamBase(bool netByteOrder = DEF_NET_BYTEORDER)
        : status_(0)
        , netByteOrder_(netByteOrder)
        , hostByteOrder_(Tools::HostByteOrder())
    {}
    bool operator !() const{return status_ != 0;}
    operator __SafeBool() const{return operator !() ? 0 : &__Myt::ResetStatus;}
    void Status(int st){status_ = st;}
    int Status() const{return status_;}
    void ResetStatus(){Status(0);}
    bool GetByteOrder() const{return netByteOrder_;}
    void SetByteOrder(bool netByteOrder){netByteOrder_ = netByteOrder;}
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{status_="<<status_
            <<", netByteOrder_="<<netByteOrder_
            <<"}";
        return oss.str();
    }
protected:
    //from, to: true-Net Byte Order, false-Host Byte Order
    //return:
    //  true    need swap byte order
    //  false   no need
    inline bool NeedReverse(bool from, bool to) const{
        return (from == to) && hostByteOrder_;
    }
private:
    int status_;
    bool netByteOrder_;
    const bool hostByteOrder_;
};

template<typename Char>
inline void __buf_copy(Char * dst, const Char * src, size_t sz)
{
    typedef std::char_traits<Char> __Traits;
    assert(sz);
    if(1 == sz)
        __Traits::assign(*dst, *src);
    else
        __Traits::copy(dst, src, sz);
}

template<typename Char>
inline void __buf_move(Char * dst, const Char * src, size_t sz)
{
    typedef std::char_traits<Char> __Traits;
    assert(sz);
    if(1 == sz)
        __Traits::assign(*dst, *src);
    else
        __Traits::move(dst, src, sz);
}

template<class Buf>
class __buf_ref_data
{
public:
    typedef Buf __Buf;
    typedef typename __Buf::value_type __Char;
private:
    typedef __buf_ref_data<__Buf> __Myt;
public:
    explicit __buf_ref_data(__Buf & buf)
        : buf_(buf)
        , begin_(buf.size())
        , cur_(0)
    {}
    __buf_ref_data(__Buf & buf, size_t begin)
        : buf_(buf)
        , begin_(begin)
        , cur_(0)
    {}
    size_t cur() const{return cur_;}
    __Char * buf(size_t i){return &buf_[offset(i)];}
    bool ensure(size_t len){
        const size_t old = buf_.size();
        if(offset(cur_) + len > old)
            buf_.resize(old + (old >> 1) + len);
        return true;
    }
    bool seek(ssize_t offset){
        if(offset < 0)
            return false;
        if(size_t(offset) > cur_)
            ensure(offset - cur_);
        cur_ = offset;
        return true;
    }
    void append(const __Char * buf, size_t sz){
        insert(cur_, buf, sz);
    }
    void insert(size_t off, const __Char * buf, size_t sz){
        assert(off >= 0 && off <= cur_);
        assert(sz && offset(cur_) + sz <= buf_.size());
        assert(buf);
        size_t c = cur_ - off;
        if(c)
            __buf_move(&buf_[offset(off + sz)], &buf_[offset(off)], c);
        __buf_copy(&buf_[offset(off)], buf, sz);
        cur_ += sz;
    }
    bool exportData(){
        buf_.resize(offset(cur_));
        cur_ = 0;
        return true;
    }
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{begin_="<<begin_
            <<", buf=("<<(cur_ + begin_)<<")"
            <<Tools::DumpHex(&buf_[0], begin_, ' ', false)
            <<" | "
            <<Tools::DumpHex(&buf_[begin_], cur_, ' ', false)
            <<"}";
        return oss.str();
    }
private:
    size_t offset(size_t i) const{return begin_ + i;}
    //members
    __Buf & buf_;
    const size_t begin_;
    size_t cur_;
};

template<class Buf>
class __buf_data
{
public:
    typedef Buf __Buf;
    typedef typename __Buf::value_type __Char;
private:
    typedef __buf_ref_data<__Buf> __Ref;
    typedef __buf_data<__Buf> __Myt;
public:
    explicit __buf_data(size_t reserve)
        : buf_(reserve, 0)
        , ref_(buf_, 0)
    {}
    size_t cur() const{return ref_.cur();}
    __Char * buf(size_t i){return ref_.buf(i);}
    bool ensure(size_t len){return ref_.ensure(len);}
    bool seek(ssize_t offset){return ref_.seek(offset);}
    void append(const __Char * buf, size_t sz){ref_.append(buf, sz);}
    void insert(size_t offset, const __Char * buf, size_t sz){ref_.insert(offset, buf, sz);}
    bool exportData(__Buf & buf){
        ref_.exportData();
        if(buf.empty()){
            buf.swap(buf_);
        }else
            exportAppend(buf);
        return true;
    }
    template<class BufT>
    bool exportData(BufT & buf){
        ref_.exportData();
        exportAppend(buf);
        return true;
    }
    template<typename CharT>
    bool exportData(CharT * buf, size_t & sz){
        if(sz < cur())
            return false;
        ref_.exportData();
        __buf_copy(buf, &buf_[0], buf_.size());
        sz = buf_.size();
        return true;
    }
    std::string ToString() const{return ref_.ToString();}
private:
    template<class BufT>
    void exportAppend(BufT & buf){
        const size_t old = buf.size();
        buf.resize(old + buf_.size());
        __buf_copy(&buf[old], &buf_[0], buf_.size());
    }
    //members
    __Buf buf_;
    __Ref ref_;
};

template<typename Char>
class __buf_data<NS_SERVER::CCharBuffer<Char> >
{
public:
    typedef CCharBuffer<Char> __Buf;
    typedef typename __Buf::value_type __Char;
private:
    typedef __buf_data<__Buf> __Myt;
public:
    explicit __buf_data(__Char * buf, size_t sz)
        : buf_(buf, sz, sz)
        , cur_(0)
    {}
    size_t cur() const{return cur_;}
    __Char * buf(size_t i){return &buf_[i];}
    bool ensure(size_t len){return (cur_ + len <= buf_.size());}
    bool seek(ssize_t offset){
        if(offset < 0 || size_t(offset) > buf_.size())
            return false;
        cur_ = offset;
        return true;
    }
    void append(const __Char * buf, size_t sz){
        insert(cur_, buf, sz);
    }
    void insert(size_t off, const __Char * buf, size_t sz){
        assert(off >= 0 && off <= cur_);
        assert(sz && cur_ + sz <= buf_.size());
        assert(buf);
        size_t c = cur_ - off;
        if(c)
            __buf_move(&buf_[off + sz], &buf_[off], c);
        __buf_copy(&buf_[off], buf, sz);
        cur_ += sz;
    }
    bool exportData(size_t & sz){
        if(sz < cur())
            return false;
        sz = buf_.size();
        cur_ = 0;
        return true;
    }
    std::string ToString() const{
        std::ostringstream oss;
        oss<<"{capacity="<<buf_.capacity()
            <<", buf_="<<Tools::DumpHex(&buf_[0], cur_)
            <<"}";
        return oss.str();
    }
private:
    //members
    __Buf buf_;
    size_t cur_;
};

//manipulators
template<class T>
struct __ManipTypeTraits{static const bool CanMemcpy = false;};
template<>
struct __ManipTypeTraits<char>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<signed char>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<unsigned char>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<short>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<unsigned short>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<int>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<unsigned int>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<long>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<unsigned long>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<long long>{static const bool CanMemcpy = true;};
template<>
struct __ManipTypeTraits<unsigned long long>{static const bool CanMemcpy = true;};

template<typename LenT, class T>
class CManipulatorArrayPtr
{
    T *         c_;
    LenT      sz1_;
    LenT *    sz2_;
public:
    CManipulatorArrayPtr(T * c, LenT sz, LenT * p)
        : c_(c)
        , sz1_(sz)
        , sz2_(p)
    {}
    T * Ptr() const{return c_;}
    LenT Size1() const{return sz1_;}
    void Size2(LenT sz) const{
        if(sz2_)
            *sz2_ = sz;
    }
};

template<typename LenT, class T>
class CManipulatorArrayCont
{
    T & c_;
    LenT max_;
public:
    CManipulatorArrayCont(T & c, LenT max_size)
        : c_(c)
        , max_(max_size)
    {}
    T & Cont() const{return c_;}
    LenT Max() const{return max_;}
};

template<class T>
class CManipulatorRawPtr
{
    T *     c_;
    size_t  sz_;
public:
    CManipulatorRawPtr(T * c, size_t sz)
        : c_(c)
        , sz_(sz)
    {}
    T * Ptr() const{return c_;}
    size_t Size() const{return sz_;}
};

template<class T>
class CManipulatorRawSeqCont
{
    T & c_;
    size_t sz_;
    size_t * sz2_;
public:
    CManipulatorRawSeqCont(T & c, size_t sz, size_t * sz2)
        : c_(c)
        , sz_(sz)
        , sz2_(sz2)
    {}
    T & Cont() const{return c_;}
    size_t Size() const{return sz_;}
    size_t Size2() const{return (sz2_ ? *sz2_ : 0);}
    void Size2(size_t sz) const{
        if(sz2_)
            *sz2_ = sz;
    }
};

template<class Iter>
class CManipulatorRawRange
{
    Iter beg_,end_;
    size_t * sz_;
public:
    CManipulatorRawRange(Iter first, Iter last, size_t * sz)
        : beg_(first)
        , end_(last)
        , sz_(sz)
    {}
    Iter Begin() const{return beg_;}
    Iter End() const{return end_;}
    void Size(size_t sz) const{
        if(sz_)
            *sz_ = sz;
    }
    size_t Size() const{return (sz_ ? *sz_ : 0);}
};

class CManipulatorSetOrder
{
    bool netByteOrder_;
public:
    explicit CManipulatorSetOrder(bool netByteOrder):netByteOrder_(netByteOrder){}
    bool GetByteOrder() const{return netByteOrder_;}
};

template<class T>
class CManipulatorValueByteOrder
{
    T & val_;
    bool nb_;
public:
    CManipulatorValueByteOrder(T & val, bool netByteOrder)
        : val_(val)
        , nb_(netByteOrder)
    {}
    T & Value() const{return val_;}
    bool GetByteOrder() const{return nb_;}
};

class CManipulatorSeek
{
    ssize_t                     off_;
    CDataStreamBase::ESeekDir   dir_;
public:
    explicit CManipulatorSeek(ssize_t off,CDataStreamBase::ESeekDir dir)
        : off_(off)
        , dir_(dir)
    {}
    ssize_t Off() const{return off_;}
    CDataStreamBase::ESeekDir Dir() const{return dir_;}
};

template<class T>
class CManipulatorOffsetValue
{
    T &     val_;
    size_t  off_;
public:
    CManipulatorOffsetValue(size_t off,T & val)
        : val_(val)
        , off_(off)
    {}
    T & Value() const{return val_;}
    size_t Off() const{return off_;}
};

template<class T>
class CManipulatorInsert
{
    const T &   val_;
    size_t      off_;
public:
    CManipulatorInsert(size_t off,const T & val)
        : val_(val)
        , off_(off)
    {}
    const T & Value() const{return val_;}
    size_t Off() const{return off_;}
};

template<class T>
class CManipulatorProtobuf
{
public:
    typedef T __Msg;
    CManipulatorProtobuf(__Msg & msg, size_t sz)
        : msg_(msg)
        , sz_(sz)
    {}
    __Msg & Msg() const{return msg_;}
    size_t Size() const{return sz_;}
private:
    __Msg & msg_;
    size_t  sz_;
};

NS_IMPL_END

#endif
