#ifndef DOZERG_LOCK_HASH_TABLE_IMPL_H_20080227
#define DOZERG_LOCK_HASH_TABLE_IMPL_H_20080227

/*
    CLockHashTable的内部实现
        __hashT_node_impl
        __hash_table_read_pointer
        __hash_table_write_pointer
    History:
        20090321    修改__hash_table_pointer_base包含__hashT_node_impl指针，而不是__hashT_node_impl::data_的指针
//*/

#include <cassert>
#include <cstring>      //memset
#include <vector>
#include <Mutex.h>

NS_IMPL_BEGIN

//for CLockHashTable

//forward declaration
template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class CLockHashTable;

template<class ValueType>
struct __hashT_node_impl
{
    __hashT_node_impl * next_;
    ValueType           data_;
};

template<class Value,class LockT>
class __hash_table_pointer_base
{
    typedef __hash_table_pointer_base<Value,LockT>  __Myt;
    typedef __hashT_node_impl<Value> *              __NodePtr;
    typedef void (__Myt::*__SafeBool)();
protected:
    typedef LockT *             __LockPtr;
    typedef CLockAdapter<LockT> __Adapter;
    __hash_table_pointer_base():pv_(0),pl_(0){}
    ~__hash_table_pointer_base(){release();}
public:
    bool operator !() const{return !pv_;}
    operator __SafeBool() const{return (operator !() ? 0 : &__Myt::release);}
    void release(){
        if(pl_)
            __Adapter().Unlock(*pl_);
        pl_ = 0;
        pv_ = 0;
    }
private:
    __hash_table_pointer_base(const __Myt &);
    __Myt & operator =(const __Myt &);
protected:
    __NodePtr   pv_;
    __LockPtr   pl_;
};

template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class __hash_table_read_pointer : public __hash_table_pointer_base<Value,LockT>
{
    friend class CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>;
    typedef __hash_table_pointer_base<Value,LockT>  __MyBase;
public:
    typedef const Value * pointer;
    typedef const Value & reference;
    pointer operator ->() const{return &operator *();}
    reference operator *() const{return __MyBase::pv_->data_;}
};

template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class __hash_table_write_pointer : public __hash_table_pointer_base<Value,LockT>
{
    friend class CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>;
    typedef __hash_table_pointer_base<Value,LockT>  __MyBase;
public:
    typedef Value * pointer;
    typedef Value & reference;
    pointer operator ->(){return &operator *();}
    reference operator *(){return __MyBase::pv_->data_;}
};

template<class Value,class LockT>
class __hash_table_elem_array_base
{
    typedef __hash_table_elem_array_base<Value,LockT>   __Myt;
    typedef __hashT_node_impl<Value> *  __NodePtr;
    typedef std::vector<__NodePtr>      __Array;
protected:
    typedef LockT *             __LockPtr;
    typedef CLockAdapter<LockT> __Adapter;
    __hash_table_elem_array_base():pl_(0){}
    ~__hash_table_elem_array_base(){release();}
public:
    void release(){
        if(pl_)
            __Adapter().Unlock(*pl_);
        pl_ = 0;
        ar_.clear();
    }
    size_t size() const{return ar_.size();}
    bool empty() const{return ar_.empty();}
private:
    __hash_table_elem_array_base(const __Myt &);
    __Myt & operator =(const __Myt &);
protected:
    __Array     ar_;
    __LockPtr   pl_;
};

template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class __hash_table_read_elem_array : public __hash_table_elem_array_base<Value,LockT>
{
    friend class CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>;
    typedef __hash_table_elem_array_base<Value,LockT>  __MyBase;
public:
    typedef const Value * pointer;
    typedef const Value & reference;
    reference operator [](size_t index) const{
        assert(__MyBase::ar_[index]);
        return __MyBase::ar_[index]->data_;
    }
};

template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class __hash_table_write_elem_array : public __hash_table_elem_array_base<Value,LockT>
{
    friend class CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>;
    typedef __hash_table_elem_array_base<Value,LockT>  __MyBase;
public:
    typedef Value * pointer;
    typedef Value & reference;
    reference operator [](size_t index){
        assert(__MyBase::ar_[index]);
        return __MyBase::ar_[index]->data_;
    }
};


//for CMulRowHashTable

struct __mr_hash_table_head
{
    template<class Primes>
    void Init(int ver, U64 row, U64 col, const Primes & primes)
    {
        assert(row > 0 && col > 0 && row == primes.size());
        ver_ = ver;
        reserved1_ = 0;
        reserved2_ = 0;
        row_ = row;
        col_ = col;
        sum_ = std::accumulate(primes.begin(), primes.end(), 0);
        used_ = 0;
        memset(reserved3_, 0, sizeof reserved3_);
        std::copy(primes.begin(), primes.end(), primes_);
    }
    bool Check(int ver, U64 row, U64 col) const{
        if(ver != ver_)
            return false;   //version mismatch
        if(!row)
            row = row_;
        if(!col)
            col = col_;
        if(!row || row != row_
                || !col || col != col_)
            return false;   //row/col mismatch
        U64 sum = std::accumulate(primes_, primes_ + row, 0);
        if(sum != sum_ || sum <= row)
            return false;
        if(used_ > sum)
            return false;
        return true;
    }
    U64 Row() const{return row_;}
    U64 Col() const{return col_;}
    U64 Sum() const{return sum_;}
    U64 Used() const{return used_;}
    void Used(ssize_t sz){used_ += sz;}
    U64 Prime(size_t index) const{return primes_[index];}
private:
    U16 ver_;
    U16 reserved1_;
    U32 reserved2_;
    U64 row_;
    U64 col_;
    U64 sum_;
    U64 used_;
    U64 reserved3_[11];
    U64 primes_[];   //row_个素数
};

NS_IMPL_END

#endif
