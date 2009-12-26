#ifndef DOZERG_DATA_STREAM_IMPL_H_20081016
#define DOZERG_DATA_STREAM_IMPL_H_20081016

#include <netinet/in.h>         //ntohl
#include <common/impl/Config.h>
#include <common/impl/Template.h>

NS_IMPL_BEGIN

class CDataStreamStatus
{
    typedef CDataStreamStatus __Myt;
    typedef void (__Myt::*__SafeBool)();
public:
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
    CDataStreamStatus():status_(0){}
    bool needReverse(bool netOrder) const{
        return (netOrder && ntohl(1) != 1);
    }
private:
    int status_;
};

//游标类
//Integer是表示游标数值的类型
//Bits表示使用多少位表示比特游标
//注意：sizeof(Integer) * 8 > Bits
template<typename Integer,int Bits>
class CStreamIndex
{
    //generate compile error if (sizeof(Integer) * 8 <= Bits)
    typedef typename NS_IMPL::CAssert<
        (Bits >= 0 && (sizeof(Integer) * 8) > Bits)>::Result __Assert;
public:
    explicit CStreamIndex(const Integer & index)
        : index_(index)
    {}
    CStreamIndex(size_t high,int low)
        : index_((high << Bits) + low)
    {}
private:
    Integer index_;
};

//manipulators
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

template<typename T>
class CManipulatorBits
{
    int len_;
    T & val_;
public:
    CManipulatorBits(int bits,T & val)
        : len_(bits)
        , val_(val)
    {}
    T & Value() const{return val_;}
    int Bits() const{return len_;}
};

NS_IMPL_END

#endif
