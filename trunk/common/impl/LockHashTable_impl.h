#ifndef DOZERG_LOCK_HASH_TABLE_IMPL_H_20080227
#define DOZERG_LOCK_HASH_TABLE_IMPL_H_20080227

/*
    CLockHashTable的内部实现
        __hashT_node_impl
        __hash_table_read_pointer
        __hash_table_write_pointer
//*/

#include <common/Mutex.h>

NS_IMPL_BEGIN

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
    typedef __hash_table_pointer_base<Value,LockT> __Myt;
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
    Value *     pv_;
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
    pointer operator ->() const{return __MyBase::pv_;}
    reference operator *() const{return *__MyBase::pv_;}
};

template<class Value,class LockT,class KeyOfValue,template<typename>class EqualKey,template<typename>class Hash,class Alloc>
class __hash_table_write_pointer : public __hash_table_pointer_base<Value,LockT>
{
    friend class CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>;
    typedef __hash_table_pointer_base<Value,LockT>  __MyBase;
public:
    typedef Value * pointer;
    typedef Value & reference;
    pointer operator ->() const{return __MyBase::pv_;}
    reference operator *() const{return *__MyBase::pv_;}
};

NS_IMPL_END

#endif
