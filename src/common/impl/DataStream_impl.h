#ifndef DOZERG_DATA_STREAM_IMPL_H_20081016
#define DOZERG_DATA_STREAM_IMPL_H_20081016

#include <algorithm>
#include <arpa/inet.h>          //ntohl
#include "Config.h"

NS_IMPL_BEGIN

class CDataStreamBase
{
    typedef CDataStreamBase __Myt;
    typedef void (__Myt::*__SafeBool)();
protected:
    typedef U16 __Length;   // type of length field (string length, array length, etc.)
public:
    enum EByteOrder{
        BigEndian,
        LittleEndian
    };
    enum EOrderType{
        NetOrder,
        HostOrder
    };
    enum ESeekDir{
        Begin,
        End,
        Cur
    };
    bool operator !() const{return status_ != 0;}
    operator __SafeBool() const{return operator !() ? 0 : &__Myt::ResetStatus;}
    void Status(int st){status_ = st;}
    int Status() const{return status_;}
    void ResetStatus(){Status(0);}
protected:
    CDataStreamBase():status_(0){}
    virtual ~CDataStreamBase(){}
    bool NeedReverse(bool netOrder) const{
        return (netOrder && ntohl(1) != 1);
    }
    bool NeedReverse(EOrderType ot) const{
        return NeedReverse(ot == NetOrder);
    }
private:
    int status_;
};

template<typename Char>
class __byte_buf_wrap
{
    typedef __byte_buf_wrap<Char> __Myt;
public:
    typedef Char __Char;
    __byte_buf_wrap()
        : buf_(NULL)
        , capa_(0)
        , sz_(0)
    {}
    __byte_buf_wrap(__Char * buf, size_t capacity, size_t size = 0)
        : buf_(buf)
        , capa_(capacity)
        , sz_(size)
    {}
    void init(__Char * buf, size_t capacity, size_t size = 0){
        buf_ = buf;
        capa_ = capacity;
        sz_ = size;
    }
    size_t size() const{return sz_;}
    size_t capacity() const{return capa_;}
    __Char & operator [](size_t index){return buf_[index];}
    const __Char & operator [](size_t index) const{return buf_[index];}
    void resize(size_t sz, __Char val = 0){
        if(sz <= capa_){
            if(sz > sz_)
                std::fill_n(buf_ + sz_, sz - sz_, val);
            sz_ = sz;
        }else{
            assert(sz < capa_);
        }
    }
    void append(const __Char * buf, size_t sz){insert(sz_, buf, sz);}
    void insert(size_t offset, const __Char * buf, size_t sz){
        if(offset <= sz_){
            if(sz_ + sz <= capa_){
                if(offset < sz_)
                    std::copy_backward(buf_ + offset, buf_ + sz_, buf_ + (sz_ + sz));
                std::copy(buf, buf + sz, buf_ + offset);
                sz_ += sz;
            }else{
                assert(sz_ + sz <= capa_);
            }
        }else{
            assert(offset <= sz_);
        }
    }

private:
    __Char * buf_;
    size_t capa_;
    size_t sz_;
};

template<class Buf>
class __buf_adapter
{
    typedef __buf_adapter<Buf>  __Myt;
    typedef Buf                 __Buf;
public:
    typedef typename __Buf::value_type   __Char;
    explicit __buf_adapter(__Buf & buf)
        : buf_(buf)
        , begin_(0)
    {}
    void init(){begin_ = buf_.size();}
    __Char & operator [](size_t index){return buf_[index];}
    const __Char & operator [](size_t index) const{return buf_[index];}
    size_t size() const{return buf_.size();}
    bool ensureRoom(ssize_t sz){
        size_t cur = buf_.size();
        if(sz < 0 && size_t(-sz) + begin_ > cur)
            return false;
        size_t old = buf_.capacity();
        if(cur + sz > old)
            buf_.reserve(old + (old >> 1) + sz);
        return true;
    }
    bool ensureSize(size_t sz){
        size_t cur = buf_.size();
        if(sz > cur)
            return ensureRoom(sz - cur);
        return true;
    }
    void resize(size_t sz){buf_.resize(sz);}
    void append(const __Char * buf, size_t sz){
        buf_.insert(buf_.end(), buf, buf + sz);
    }
    void insert(size_t offset, const __Char * buf, size_t sz){
        buf_.insert(buf_.begin() + offset, buf, buf + sz);
    }
    bool exportData(__Buf & buf){
        if(buf.empty())
            buf.swap(buf_);
        else
            buf.insert(buf.end(), buf_.begin(), buf_.end());
        return true;
    }
    template<typename __Char>
    bool exportData(__Char * buf, size_t & sz) const{
        if(sz < buf_.size())
            return false;
        memcpy(buf, &buf_[0], buf_.size());
        sz = buf_.size();
        return true;
    }
private:
    __buf_adapter(const __Myt &);
    __Myt & operator =(const __Myt &);
    __Buf & buf_;
    size_t begin_;
};

template<typename Char>
class __buf_adapter<__byte_buf_wrap<Char> >
{
    typedef __buf_adapter<__byte_buf_wrap<Char> >   __Myt;
    typedef __byte_buf_wrap<Char>                   __Buf;
public:
    typedef Char                                    __Char;
    explicit __buf_adapter(__Buf & buf)
        : buf_(buf)
        , begin_(0)
    {}
    void init(){begin_ = buf_.size();}
    __Char & operator [](size_t index){return buf_[index];}
    const __Char & operator [](size_t index) const{return buf_[index];}
    size_t size() const{return buf_.size();}
    bool ensureRoom(ssize_t sz){
        size_t cur = buf_.size();
        if(sz < 0 && size_t(-sz) + begin_ > cur)
            return false;
        return (cur + sz <= buf_.capacity());
    }
    bool ensureSize(size_t sz){
        size_t cur = buf_.size();
        if(sz > cur)
            return ensureRoom(sz - cur);
        return true;
    }
    void resize(size_t sz){buf_.resize(sz);}
    void append(const __Char * buf, size_t sz){
        buf_.append(buf, sz);
    }
    void insert(size_t offset, const __Char * buf, size_t sz){
        buf_.insert(offset, buf, sz);
    }
    template<typename CharT>
    bool exportData(CharT * buf, size_t & sz) const{
        if(sz < buf_.size())
            return false;
        memcpy(buf, &buf_[0], buf_.size());
        sz = buf_.size();
        return true;
    }
private:
    __buf_adapter(const __Myt &);
    __Myt & operator =(const __Myt &);
    __Buf & buf_;
    size_t begin_;
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

template<typename T>
class CManipulatorArray
{
    T *         c_;
    size_t      sz1_;
    size_t *    sz2_;
public:
    CManipulatorArray(T * c,size_t sz,size_t * p)
        : c_(c)
        , sz1_(sz)
        , sz2_(p)
    {}
    T * Ptr() const{return c_;}
    size_t Size1() const{return sz1_;}
    void Size2(size_t sz) const{
        if(sz2_)
            *sz2_ = sz;
    }
};

template<typename T>
class CManipulatorRaw
{
    T *     c_;
    size_t  sz_;
public:
    CManipulatorRaw(T * c,size_t sz):c_(c),sz_(sz){}
    T * Ptr() const{return c_;}
    size_t Size() const{return sz_;}
};

template<typename Iter>
class CManipulatorRange
{
    Iter beg_,end_;
public:
    CManipulatorRange(Iter first,Iter last)
        : beg_(first)
        , end_(last)
    {}
    Iter Begin() const{return beg_;}
    Iter End() const{return end_;}
};

class CManipulatorSetOrder
{
    CDataStreamBase::EOrderType order_;
public:
    explicit CManipulatorSetOrder(CDataStreamBase::EOrderType od):order_(od){}
    CDataStreamBase::EOrderType Order() const{return order_;}
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
