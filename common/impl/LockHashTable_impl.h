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

#include <vector>
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
    typedef __hashT_node_impl<Value>    __NodePtr;
    typedef __DZ_VECTOR(__NodePtr)      __Array;
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

NS_IMPL_END

#endif
