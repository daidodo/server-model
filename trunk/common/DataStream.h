#ifndef DOZERG_DATA_STREAM_H_20070905
#define DOZERG_DATA_STREAM_H_20070905

/*
    封装数据流的读取和写入
    注意测试operator !(),在错误状态下,所有读写数据操作都会无效
        CInByteStream       以字节为单位的输入流
        COutByteStream      以字节为单位的输出流
        array               输入/输出数组
        raw                 输入/输出数组数据
        range               输入/输出范围
        set_order           设置输入/输出流的字节序
        seek                设置输入/输出流的偏移
        offset_value        输入/输出指定位置的数据
    History
        20070926    给CInByteStream加入status_状态,防止因非法数据引起内存访问越界
        20071228    修正CInByteStream::LeftSize在len < cur_时返回很大size_t的问题;并把2个ensure里的减法改成加法
        20080204    去掉CInByteStream::DumpLeft,加入ToString,输出对象内部状态
        20081008    将CInDataStream和COutDataStream更名为CInByteStream和COutByteStream
        20081016    调整类结构，引入CDataStreamBase作为所有数据流基类
                    重写CInByteStream和COutByteStream，加入更多类型支持
                    引入manipulator，并实现多种输入输出方式
        20081106    增加Manip::insert
    Manual:
        请参考"docs/DataStream-manual.txt"
    
//*/

#include <cassert>
#include <string>
#include <vector>
#include <common/impl/DataStream_impl.h>

NS_SERVER_BEGIN

class CInByteStream : public NS_IMPL::CDataStreamBase
{
    typedef CInByteStream __Myt;
    static const bool DEF_NET_BYTEORDER = true;    //默认使用网络字节序(true)还是本地字节序(false)
    const char *    data_;
    size_t          len_;
    size_t          cur_;
    bool            need_reverse_;  //是否需要改变字节序
public:
    CInByteStream(const char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){   //netByteOrder表示是否按网络字节序
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const unsigned char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const signed char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,l,netByteOrder);
    }
    CInByteStream(const __DZ_VECTOR(char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    CInByteStream(const __DZ_VECTOR(unsigned char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    CInByteStream(const __DZ_VECTOR(signed char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    void SetSource(const char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        data_ = d;
        len_ = l;
        cur_ = 0;
        need_reverse_ = NeedReverse(netByteOrder);
        ResetStatus();
    }
    void SetSource(const unsigned char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)d,l,netByteOrder);
    }
    void SetSource(const signed char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)d,l,netByteOrder);
    }
    void SetSource(const __DZ_VECTOR(char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(&d[0],d.size(),netByteOrder);
    }
    void SetSource(const __DZ_VECTOR(unsigned char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)&d[0],d.size(),netByteOrder);
    }
    void SetSource(const __DZ_VECTOR(signed char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource((const char *)&d[0],d.size(),netByteOrder);
    }
    //设置字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    //按照dir指定的方向设置cur_指针偏移
    //返回cur_最后的绝对偏移
    size_t Seek(ssize_t off,ESeekDir dir){
        switch(dir){
            case Begin:
                cur_ = off;
                break;
            case End:
                assert(size_t(off) <= len_);
                cur_ = len_ - off;
                break;
            case Cur:
                cur_ += off;
                break;
            default:
                assert(0);
        }
        return cur_;
    }
    //返回当前cur_指针的偏移，dir表示相对位置
    size_t Tell(ESeekDir dir = Begin) const{
        switch(dir){
            case Begin:
                return cur_;
            case End:
                return (len_ > cur_ ? len_ - cur_ : 0);
            default:;
        }
        return 0;
    }
    size_t CurPos() const{return Tell();}
    //剩余的字节数
    size_t LeftSize() const{return Tell(End);}
    //read PODs
    __Myt & operator >>(char & c)               {return readPod(c);}
    __Myt & operator >>(signed char & c)        {return readPod(c);}
    __Myt & operator >>(unsigned char & c)      {return readPod(c);}
    __Myt & operator >>(short & c)              {return readPod(c);}
    __Myt & operator >>(unsigned short & c)     {return readPod(c);}
    __Myt & operator >>(int & c)                {return readPod(c);}
    __Myt & operator >>(unsigned int & c)       {return readPod(c);}
    __Myt & operator >>(long & c)               {return readPod(c);}
    __Myt & operator >>(unsigned long & c)      {return readPod(c);}
    __Myt & operator >>(long long & c)          {return readPod(c);}
    __Myt & operator >>(unsigned long long & c) {return readPod(c);}
    //read std::string
    __Myt & operator >>(__DZ_STRING & c){
        U32 sz;
        operator >>(sz);
        if(ensure(sz)){
            c.assign(data_ + cur_ ,data_ + cur_ + sz);
            cur_ += sz;
        }
        return *this;
    }
    //read array( = length + raw array)
    template<class T>
    __Myt & operator >>(T * c){
        U32 sz;
        operator >>(sz);
        return readRaw(c,sz);
    }
    //read array( = length + raw array) through CManipulatorArray
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorArray<T> & m){
        U32 sz;
        operator >>(sz);
        if(sz > m.Size1()){
            Status(1);
            return *this;
        }
        m.Size2(sz);
        return readRaw(m.Ptr(),sz);
    }
    //read raw array through CManipulatorRaw
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorRaw<T> & m){
        return readRaw(m.Ptr(),m.Size());
    }
    //read range of raw array through CManipulatorRange
    template<class Iter>
    __Myt & operator >>(const NS_IMPL::CManipulatorRange<Iter> & m){
        for(Iter i = m.Begin();i != m.End();++i)
            if(!(*this>>(*i)))
                break;
        return *this;
    }
    //set order type(NetOrder or HostOrder) through CManipulatorSetOrder
    __Myt & operator >>(const NS_IMPL::CManipulatorSetOrder & m){
        OrderType(m.Order());
        return *this;
    }
    //set cur_ position through CManipulatorSeek
    __Myt & operator >>(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(),m.Dir());
        return *this;
    }
    //read value from a particular position but not change cur_
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = cur_;
        Seek(m.Off(),Begin);
        *this>>(m.Value());
        Seek(old,Begin);
        return *this;
    }
private:
    template<typename T>
    __Myt & readPod(T & c){
        if(ensure(sizeof(T))){
            memcpy(&c,data_ + cur_,sizeof(T));
            if(need_reverse_ && sizeof(T) > 1)
                c = Tools::SwapByteOrder(c);
            cur_ += sizeof(T);
        }
        return *this;
    }
    template<typename T>
    __Myt & readRaw(T * c,size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && need_reverse_))
        {
            for(size_t i = 0;i < sz;++i,++c)
                if(!(*this>>(*c)))
                    break;
        }else{
            sz *= sizeof(T);
            if(ensure(sz)){
                memcpy(c,data_ + cur_,sz);
                cur_ += sz;
            }
        }
        return *this;
    }
    template<typename T>
    __Myt & readArray(T * c){
        U32 sz;
        operator >>(sz);
        return readRaw(c,sz);
    }
    bool ensure(size_t sz){     //防止越界访问data_
        if(operator !())
            return false;
        if(len_ < cur_ + sz){
            Status(1);
            return false;
        }
        return true;
    }
};

class COutByteStream : public NS_IMPL::CDataStreamBase
{
    typedef COutByteStream __Myt;
    static const bool DEF_NET_BYTEORDER = true;    //默认使用网络字节序(true)还是本地字节序(false)
    __DZ_VECTOR(char)   data_;
    size_t              cur_;
    bool                need_reverse_;  //是否需要改变结果的byte order
public:
    explicit COutByteStream(size_t sz = 100,bool netByteOrder = DEF_NET_BYTEORDER)
        : data_(sz)
        , cur_(0)
        , need_reverse_(NeedReverse(netByteOrder))
    {}
    //设置/获取字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    EOrderType OrderType() const{return (need_reverse_ ? HostOrder : NetOrder);}
    //按照dir指定的方向设置cur_指针偏移
    //返回cur_最后的绝对偏移
    //注意：如果cur_变小，相当于抹掉了cur_之后的数据；如果cur_变大了，相当于留出指定的空位
    size_t Seek(ssize_t off,ESeekDir dir){
        switch(dir){
            case Begin:
                assert(off >= 0);
                if(size_t(off) > cur_)
                    ensure(size_t(off) - cur_);
                cur_ = off;
                break;
            case End:
            case Cur:
                if(off > 0)
                    ensure(off);
                else
                    assert(size_t(-off) < cur_);
                cur_ += off;
                break;
            default:
                assert(0);
        }
        return cur_;
    }
    //返回当前cur_指针的绝对偏移
    size_t Tell() const{return cur_;}
    size_t Size() const{return Tell();}
    //导出所有写入的数据
    //bAppend表示是追加到dst已有数据后面，还是覆盖dst原有的数据
    bool ExportData(__DZ_STRING & dst,bool bAppend = false){
        data_.resize(cur_);
        if(bAppend){    //数据加到dst后面
            dst.insert(dst.end(),data_.begin(),data_.end());
        }else{          //覆盖dst原有数据
            dst.assign(data_.begin(),data_.end());
        }
        cur_ = 0;
        return true;
    }
    bool ExportData(__DZ_VECTOR(char) & dst,bool bAppend = false){
        data_.resize(cur_);
        if(bAppend){    //数据加到dst后面
            dst.insert(dst.end(),data_.begin(),data_.end());
        }else{          //覆盖dst原有数据
            data_.swap(dst);
        }
        cur_ = 0;
        return true;
    }
    bool ExportData(char * dst,size_t & sz){
        if(sz < data_.size())
            return false;
        sz = data_.size();
        if(sz > 0){
            memcpy(dst,&data_[0],sz);
        }
        cur_ = 0;
        return true;
    }
    //write PODs
    __Myt & operator <<(char c)                 {return writePod(c);}
    __Myt & operator <<(signed char c)          {return writePod(c);}
    __Myt & operator <<(unsigned char c)        {return writePod(c);}
    __Myt & operator <<(short c)                {return writePod(c);}
    __Myt & operator <<(unsigned short c)       {return writePod(c);}
    __Myt & operator <<(int c)                  {return writePod(c);}
    __Myt & operator <<(unsigned int c)         {return writePod(c);}
    __Myt & operator <<(long c)                 {return writePod(c);}
    __Myt & operator <<(unsigned long c)        {return writePod(c);}
    __Myt & operator <<(long long c)            {return writePod(c);}
    __Myt & operator <<(unsigned long long c)   {return writePod(c);}
    //write std::string
    __Myt & operator <<(__DZ_STRING c){
        return writeArray(c.c_str(),c.length());
    }
    //write array( = length + raw array) through CManipulatorArray
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorArray<T> & m){
        return writeArray(m.Ptr(),m.Size1());
    }
    //write raw array through CManipulatorRaw
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorRaw<T> & m){
        return writeRaw(m.Ptr(),m.Size());
    }
    //write range of raw array through CManipulatorRange
    template<class Iter>
    __Myt & operator <<(const NS_IMPL::CManipulatorRange<Iter> & m){
        for(Iter i = m.Begin();i != m.End();++i)
            *this<<(*i);
        return *this;
    }
    //set order type(NetOrder, HostOrder) through CManipulatorSetOrder
    __Myt & operator <<(const NS_IMPL::CManipulatorSetOrder & m){
        OrderType(m.Order());
        return *this;
    }
    //set cur_ position through CManipulatorSeek
    __Myt & operator <<(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(),m.Dir());
        return *this;
    }
    //write value to a particular position but not change cur_
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = cur_;
        Seek(m.Off(),Begin);
        *this<<(m.Value());
        Seek(old,Begin);
        return *this;
    }
    //insert value into a particular position and change cur_ relatively
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorInsert<T> & m){
        __Myt ds;
        ds.need_reverse_ = need_reverse_;
        if(ds<<m.Value()){
            __DZ_VECTOR(char) tmp;
            ds.ExportData(tmp);
            data_.insert(data_.begin() + m.Off(),tmp.begin(),tmp.end());
            cur_ += tmp.size();
        }
        return *this;
    }
private:
    template<typename T>
    __Myt & writePod(T c){
        ensure(sizeof(T));
        if(need_reverse_ && sizeof(T) > 1)
            c = Tools::SwapByteOrder(c);
        memcpy(&data_[cur_],&c,sizeof(T));
        cur_ += sizeof(T);
        return *this;
    }
    template<typename T>
    __Myt & writeRaw(const T * c,size_t sz){
        assert(c);
        if(!NS_IMPL::__ManipTypeTraits<T>::CanMemcpy
            || (sizeof(T) > 1 && need_reverse_))
        {
            for(size_t i = 0;i < sz;++i,++c)
                *this<<(*c);
        }else{
            sz *= sizeof(T);
            ensure(sz);
            memcpy(&data_[cur_],c,sz);
            cur_ += sz;
        }
        return *this;
    }
    template<typename T>
    __Myt & writeArray(const T * c,size_t sz){
        assert(c);
        operator <<(U32(sz));
        return writeRaw(c,sz);
    }
    void ensure(size_t len){
        size_t curLen = data_.size();
        if(curLen < len + cur_)
            data_.resize(curLen + (curLen >> 1) + len);
    }
};

//manipulators' functions:
namespace Manip{
    //write array( = length + raw array)
    template<class T>
    inline NS_IMPL::CManipulatorArray<T> array(T * c,size_t sz,size_t * real_sz = 0){
        return NS_IMPL::CManipulatorArray<T>(c,sz,real_sz);
    }

    //read/write raw array
    template<class T>
    inline NS_IMPL::CManipulatorRaw<T> raw(T * c,size_t sz){
        return NS_IMPL::CManipulatorRaw<T>(c,sz);
    }

    //read/write range [first,last) of raw array
    template<class Iter>
    inline NS_IMPL::CManipulatorRange<Iter> range(Iter first,Iter last){
        return NS_IMPL::CManipulatorRange<Iter>(first,last);
    }

    //set byte order type(NetOrder or HostOrder)
    inline NS_IMPL::CManipulatorSetOrder set_order(NS_IMPL::CDataStreamBase::EOrderType order){
        return NS_IMPL::CManipulatorSetOrder(order);
    }

    inline NS_IMPL::CManipulatorSetOrder set_order(bool netByteOrder){
        return NS_IMPL::CManipulatorSetOrder(
            netByteOrder ? NS_IMPL::CDataStreamBase::NetOrder : NS_IMPL::CDataStreamBase::HostOrder);
    }

    //set read/write position
    inline NS_IMPL::CManipulatorSeek seek(ssize_t off,NS_IMPL::CDataStreamBase::ESeekDir dir = NS_IMPL::CDataStreamBase::Begin){
        return NS_IMPL::CManipulatorSeek(off,dir);
    }

    //skip/reserve certain bytes
    inline NS_IMPL::CManipulatorSeek skip(size_t off){
        return NS_IMPL::CManipulatorSeek(off,NS_IMPL::CDataStreamBase::Cur);
    }

    //read/write value from offset position
    template<class T>
    inline NS_IMPL::CManipulatorOffsetValue<T> offset_value(size_t offset,T & val){
        return NS_IMPL::CManipulatorOffsetValue<T>(offset,val);
    }
    template<class T>
    inline NS_IMPL::CManipulatorOffsetValue<const T> offset_value(size_t offset,const T & val){
        return NS_IMPL::CManipulatorOffsetValue<const T>(offset,val);
    }

    //insert value into offset position
    template<class T>
    inline NS_IMPL::CManipulatorInsert<T> insert(size_t offset,const T & val){
        return NS_IMPL::CManipulatorInsert<T>(offset,val);
    }

}//namespace Manip

NS_SERVER_END

#endif
