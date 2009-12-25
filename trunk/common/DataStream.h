#ifndef DOZERG_DATA_STREAM_H_20070905
#define DOZERG_DATA_STREAM_H_20070905

/*
    封装数据流的读取和写入
    注意测试operator !(),在错误状态下,所有读写数据操作都会无效
    类型:
        CInByteStream       以字节为单位的输入流
        COutByteStream      以字节为单位的输出流
        CInBitStream        以比特位为单位的输入流
        COutBitStream       以比特位为单位的输出流
    操作符:
        array               输入/输出数组
        raw                 输入/输出数组数据
        range               输入/输出范围
        set_order           设置输入/输出流的字节序
        seek                设置输入/输出流的偏移
        skip                跳过/预留指定字节数据
        offset_value        输入/输出指定位置的数据
        insert              在指定位置插入数据
    History
        20070926    给CInByteStream加入status_状态,防止因非法数据引起内存访问越界
        20071228    修正CInByteStream::LeftSize在len < bytePos_时返回很大size_t的问题;并把2个ensure里的减法改成加法
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
#include <algorithm>    //std::reverse
#include <common/impl/DataStream_impl.h>
#include <common/Tools.h>

NS_SERVER_BEGIN

class CInByteStream : public NS_IMPL::CDataStreamBase
{
    typedef CInByteStream __Myt;
protected:
    const char *    data_;
    size_t          len_;
    size_t          bytePos_;
    bool            need_reverse_;  //是否需要改变字节序
    CInByteStream(){}
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
    explicit CInByteStream(const __DZ_VECTOR(char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    explicit CInByteStream(const __DZ_VECTOR(unsigned char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    explicit CInByteStream(const __DZ_VECTOR(signed char) & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    explicit CInByteStream(const __DZ_STRING & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d,netByteOrder);
    }
    void SetSource(const char * d,size_t l,bool netByteOrder = DEF_NET_BYTEORDER){
        data_ = d;
        len_ = l;
        bytePos_ = 0;
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
    void SetSource(const __DZ_STRING & d,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(d.c_str(),d.size(),netByteOrder);
    }
    //设置字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    //按照dir指定的方向设置bytePos_指针偏移
    //返回bytePos_最后的绝对偏移
    size_t Seek(ssize_t off,ESeekDir dir){
        switch(dir){
            case Begin:
                bytePos_ = off;
                break;
            case End:
                assert(size_t(off) <= len_);
                bytePos_ = len_ - off;
                break;
            case Cur:
                bytePos_ += off;
                break;
            default:
                assert(0);
        }
        return bytePos_;
    }
    //返回当前bytePos_指针的偏移，dir表示相对位置
    size_t Tell(ESeekDir dir = Begin) const{
        switch(dir){
            case Begin:
                return bytePos_;
            case End:
                return (len_ > bytePos_ ? len_ - bytePos_ : 0);
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
            c.assign(data_ + bytePos_ ,data_ + bytePos_ + sz);
            bytePos_ += sz;
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
    //set bytePos_ position through CManipulatorSeek
    __Myt & operator >>(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(),m.Dir());
        return *this;
    }
    //read value from a particular position but not change bytePos_
    template<class T>
    __Myt & operator >>(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = bytePos_;
        Seek(m.Off(),Begin);
        *this>>(m.Value());
        Seek(old,Begin);
        return *this;
    }
private:
    template<typename T>
    __Myt & readPod(T & c){
        if(ensure(sizeof(T))){
            memcpy(&c,data_ + bytePos_,sizeof(T));
            if(need_reverse_ && sizeof(T) > 1)
                c = Tools::SwapByteOrder(c);
            bytePos_ += sizeof(T);
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
                memcpy(c,data_ + bytePos_,sz);
                bytePos_ += sz;
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
        if(len_ < bytePos_ + sz){
            Status(1);
            return false;
        }
        return true;
    }
};

class COutByteStream : public NS_IMPL::CDataStreamBase
{
    typedef COutByteStream __Myt;
protected:
    __DZ_VECTOR(char)   data_;
    size_t              bytePos_;
    bool                need_reverse_;  //是否需要改变结果的byte order
public:
    explicit COutByteStream(size_t sz = 128,bool netByteOrder = DEF_NET_BYTEORDER)
        : data_(sz)
        , bytePos_(0)
        , need_reverse_(NeedReverse(netByteOrder))
    {}
    //设置/获取字节序类型
    void OrderType(EOrderType ot){need_reverse_ = NeedReverse(ot);}
    EOrderType OrderType() const{return (need_reverse_ ? HostOrder : NetOrder);}
    //按照dir指定的方向设置bytePos_指针偏移
    //返回bytePos_最后的绝对偏移
    //注意：如果bytePos_变小，相当于抹掉了bytePos_之后的数据；如果bytePos_变大了，相当于留出指定的空位
    size_t Seek(ssize_t off,ESeekDir dir){
        switch(dir){
            case Begin:
                assert(off >= 0);
                if(size_t(off) > bytePos_)
                    ensure(size_t(off) - bytePos_);
                bytePos_ = off;
                break;
            case End:
            case Cur:
                if(off > 0)
                    ensure(off);
                else{
                    assert(size_t(-off) < bytePos_);
                }
                bytePos_ += off;
                break;
            default:
                assert(0);
        }
        return bytePos_;
    }
    //返回当前bytePos_指针的绝对偏移
    size_t Tell() const{return bytePos_;}
    size_t Size() const{return Tell();}
    //导出所有写入的数据
    //bAppend表示是追加到dst已有数据后面，还是覆盖dst原有的数据
    bool ExportData(__DZ_STRING & dst,bool bAppend = false){
        data_.resize(bytePos_);
        if(bAppend){    //数据加到dst后面
            dst.insert(dst.end(),data_.begin(),data_.end());
        }else{          //覆盖dst原有数据
            dst.assign(data_.begin(),data_.end());
        }
        bytePos_ = 0;
        return true;
    }
    bool ExportData(__DZ_VECTOR(char) & dst,bool bAppend = false){
        data_.resize(bytePos_);
        if(bAppend){    //数据加到dst后面
            dst.insert(dst.end(),data_.begin(),data_.end());
        }else{          //覆盖dst原有数据
            data_.swap(dst);
        }
        bytePos_ = 0;
        return true;
    }
    bool ExportData(char * dst,size_t & sz){
        if(sz < data_.size())
            return false;
        sz = data_.size();
        if(sz > 0){
            memcpy(dst,&data_[0],sz);
        }
        bytePos_ = 0;
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
    //set bytePos_ position through CManipulatorSeek
    __Myt & operator <<(const NS_IMPL::CManipulatorSeek & m){
        Seek(m.Off(),m.Dir());
        return *this;
    }
    //write value to a particular position but not change bytePos_
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorOffsetValue<T> & m){
        size_t old = bytePos_;
        Seek(m.Off(),Begin);
        *this<<(m.Value());
        if(old > bytePos_)
            Seek(old,Begin);
        return *this;
    }
    //insert value into a particular position and change bytePos_ relatively
    template<class T>
    __Myt & operator <<(const NS_IMPL::CManipulatorInsert<T> & m){
        __Myt ds;
        ds.need_reverse_ = need_reverse_;
        if(ds<<m.Value()){
            __DZ_VECTOR(char) tmp;
            ds.ExportData(tmp);
            data_.insert(data_.begin() + m.Off(),tmp.begin(),tmp.end());
            bytePos_ += tmp.size();
        }
        return *this;
    }
private:
    template<typename T>
    __Myt & writePod(T c){
        ensure(sizeof(T));
        if(need_reverse_ && sizeof(T) > 1)
            c = Tools::SwapByteOrder(c);
        memcpy(&data_[bytePos_],&c,sizeof(T));
        bytePos_ += sizeof(T);
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
            memcpy(&data_[bytePos_],c,sz);
            bytePos_ += sz;
        }
        return *this;
    }
    template<typename T>
    __Myt & writeArray(const T * c,size_t sz){
        operator <<(U32(sz));
        if(sz){
            assert(c);
            return writeRaw(c,sz);
        }else
            return *this;
    }
protected:
    void ensure(size_t len){
        size_t curLen = data_.size();
        if(curLen < len + bytePos_)
            data_.resize(curLen + (curLen >> 1) + len);
    }
};

class CInBitStream : public CInByteStream
{
    typedef CInByteStream   __MyBase;
    typedef CInBitStream    __Myt;
    int bitPos_;
public:
    template<typename CharType>
    CInBitStream(const CharType * data,size_t len,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(data,len,netByteOrder);
    }
    template<class CharContainer>
    explicit CInBitStream(const CharContainer & data,bool netByteOrder = DEF_NET_BYTEORDER){
        SetSource(data,netByteOrder);
    }
    template<class CharType>
    void SetSource(const CharType * data,size_t len,bool netByteOrder = DEF_NET_BYTEORDER){
        bitPos_ = 0;
        __MyBase::SetSource(data,len,netByteOrder);
    }
    template<class CharContainer>
    void SetSource(const CharContainer & data,bool netByteOrder = DEF_NET_BYTEORDER){
        bitPos_ = 0;
        __MyBase::SetSource(data,netByteOrder);
    }
    //按照dir指定的方向设置比特位指针偏移
    //返回比特位指针最后的绝对偏移
    size_t SeekBits(ssize_t off,ESeekDir dir){
        ssize_t bits = off % 8;
        off /= 8;
        switch(dir){
            case Begin:
                bytePos_ = off;
                bitPos_ = bits;
                break;
            case End:
                assert(size_t(off) + (bits > 0) <= len_);
                bytePos_ = len_ - off;
                bitPos_ -= bits;
                if(bitPos_ < 0){
                    bitPos_ += 8;
                    --bytePos_;
                }
                break;
            case Cur:
                bytePos_ += off;
                bitPos_ += bits;
                if(bitPos_ >= 8){
                    bitPos_ -= 8;
                    ++bytePos_;
                }
                break;
            default:
                assert(0);
        }
        return bytePos_ * 8 + bitPos_;
    }
    //read bytes
    template<class T>
    __Myt & operator >>(T & c){
        assert(!bitPos_);
        __MyBase::operator >>(c);
        return *this;
    }
    //read bits
    template<typename Integer>
    __Myt & operator >>(const NS_IMPL::CManipulatorBits<Integer> & m){
//*
        const int MAX_BITS = NS_IMPL::CIntegerTraits<Integer>::MAX_BITS;
        const int BITS = m.Bits();
        if(ensureBits(BITS)){
            Integer & v = (m.Value() = 0);
            int bits = BITS,lsh = 0;
            for(Integer c;bits > 0 && lsh < MAX_BITS;lsh = BITS - bits)
                v += ((c = bitsVal(bits) & 0xFF) <<= lsh);
            if(bits)
                SeekBits(bits,Cur);
        }
/*/
        Integer & v = m.Value();
        v = 0;
        Integer mask(1);
        for(int i = m.Bits();i > 0;--i,mask <<= 1){
            if(bytePos_ >= len_){
                Status(1);
                break;
            }
            if((data_[bytePos_] >> bitPos_) & 1)
                v += mask;
			if(++bitPos_ == 8)
				bitPos_ = 0,++bytePos_;
        }
//*/
        return *this;
    }

    __DZ_STRING ToString() const{
        const char DIGIT[] = "01";
        __DZ_STRING ret;
        size_t by = bytePos_;
        int bi = bitPos_;
        while(by < len_){
            if(!bi && !ret.empty())
                ret.push_back(' ');
            ret.push_back(DIGIT[(data_[by] >> bi) & 1]);
			if(++bi == 8)
				bi = 0,++by;
        }
        std::reverse(ret.begin(),ret.end());
        return ret;
    }
private:
    bool ensureBits(int bits){
        if(operator !())
            return false;
        int sz = (bits + bitPos_ - 1) / 8;
        if(sz > 0 && len_ < bytePos_ + sz){
            Status(1);
            return false;
        }
        return true;
    }
    char bitsVal(int & bits){
        int left = 8 - bitPos_;
        if(left > bits)
            left = bits;
        char ret = (data_[bytePos_] >> bitPos_) & ((1 << left) - 1);
        if((bitPos_ += left) >= 8)
            ++bytePos_,bitPos_ -= 8;
        bits -= left;
        return ret;
    }
};

class COutBitStream : public COutByteStream
{
    typedef COutByteStream __MyBase;
    typedef COutBitStream __Myt;
    int bitPos_;
public:
    explicit COutBitStream(size_t sz = 128,bool netByteOrder = DEF_NET_BYTEORDER)
        : __MyBase(sz,netByteOrder)
        , bitPos_(7)
    {}
    //按照dir指定的方向设置比特位指针偏移
    //返回比特位最后的绝对偏移
    //注意：如果比特位变小，相当于抹掉了原先的数据；如果比特位变大了，相当于留出指定的空位
    size_t SeekBits(ssize_t off,ESeekDir dir){
        int bits = off % 8;
        off = (off + 7) / 8;
        switch(dir){
            case Begin:
                assert(off >= 0);
                if(size_t(off) > bytePos_)
                    ensure(size_t(off) - bytePos_);
                bytePos_ = off;
                bitPos_ = (8 - bits) % 8;
                break;
            case End:
            case Cur:
                if(off > 0)
                    ensure(off);
                else{
                    assert(size_t(-off) < bytePos_);
                }
                bytePos_ += off;
                bitPos_ -= bits;
                if(bitPos_ < 0)
                    bitPos_ += 8;
                break;
            default:
                assert(0);
        }
        return bytePos_ * 8 - bitPos_;
    }
    //write bytes
    template<class T>
    __Myt & operator <<(const T & c){
        assert(!bitPos_);
        __MyBase::operator <<(c);
        return *this;
    }
    //write bits
    template<typename Integer>
    __Myt & operator <<(const NS_IMPL::CManipulatorBits<Integer> & m){
        const int MAX_BITS = NS_IMPL::CIntegerTraits<Integer>::MAX_BITS;
        const int BITS = m.Bits();
        ensureBits(BITS);
        int bits = BITS;
        if(BITS > MAX_BITS){
            SeekBits(BITS - MAX_BITS,Cur);
            bits = MAX_BITS;
        }
        while(bits > 0)
            bitsVal(m.Value(),bits);

        return *this;
    }
    __DZ_STRING ToString() const{
        const char DIGIT[] = "01";
        __DZ_STRING ret;
        for(size_t i = 0;i < bytePos_;++i){
            for(int j = 7;j >= 0;--j)
                ret.push_back(DIGIT[(data_[i] >> j) & 1]);
            ret.push_back(' ');
        }
        for(int i = 7;bitPos_ && i >= bitPos_;--i)
            ret.push_back(DIGIT[(data_[bytePos_] >> i) & 1]);
        return ret;
    }
private:
    void ensureBits(int bits){
        __MyBase::ensure((bits - bitPos_ + 7) / 8);
    }
    template<typename Integer>
    void bitsVal(const Integer & v,int & bits){
        if(!bitPos_){
            bitPos_ = 8;
            ++bytePos_;
        }
        int left = bitPos_;
        if(left > bits)
            left = bits;

        char mask = (1 << left) - 1;
        data_[bytePos_] &= ~mask;
        mask &= (v >> (bits - left));
        data_[bytePos_] += mask;
        bitPos_ -= left;
        bits -= left;
    }
};

//manipulators' functions:
namespace Manip{
    //read/write array( = length + raw array)
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
    inline NS_IMPL::CManipulatorSeek skip(ssize_t off){
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

    //read/write bits from bits stream
    template<typename T>
    inline NS_IMPL::CManipulatorBits<T> bits(size_t bits,T & val){
        return NS_IMPL::CManipulatorBits<T>(bits,val);
    }
    template<class T>
    inline NS_IMPL::CManipulatorBits<const T> bits(size_t bits,const T & val){
        return NS_IMPL::CManipulatorBits<const T>(bits,val);
    }

}//namespace Manip

NS_SERVER_END

#endif
