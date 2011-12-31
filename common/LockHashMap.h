#ifndef DOZERG_LOCK_HASH_SET_H_20070908
#define DOZERG_LOCK_HASH_SET_H_20070908

/*
    对桶加锁的hash set和map, 在CLockHashTable基础上实现
        CLockHashSet
        CLockHashMap
    History:
        20090316    CLockHashMap增加BucketSize和Iterate函数，提供遍历容器的途径
        20111113    修改CLockHashMap的value_type，将key_type加const修饰，避免write_pointer可以修改
//*/

#include <common/impl/Config.h>
#include <functional>           //std::equal_to
#include <utility>              //std::pair
#include <common/impl/LockHashTable.h>  //Tools::HashFn, Tools::CIdentity, Tools::CSelect1st

NS_SERVER_BEGIN

template<
    class Key,
    class LockT = CRWLock,
    template<typename>class Hash = Tools::HashFn,
    template<typename>class EqualKey = std::equal_to,
    class Alloc = std::allocator<Key>
>class CLockHashSet{
    //typedefs:
    typedef CLockHashSet<Key, LockT, Hash, EqualKey, Alloc> __Myt;
    typedef NS_IMPL::CLockHashTable<Key, LockT,
        Tools::CIdentity<Key>, EqualKey, Hash, Alloc>  __HashTable;
public:
    typedef typename __HashTable::key_type          key_type;
    typedef typename __HashTable::value_type        value_type;
    typedef typename __HashTable::lock_type         lock_type;
    typedef typename __HashTable::key_equal         key_equal;
    typedef typename __HashTable::hasher            hasher;
    typedef typename __HashTable::allocator_type    allocator_type;
    typedef typename __HashTable::pointer           pointer;
    typedef typename __HashTable::const_pointer     const_pointer;
    typedef typename __HashTable::reference         reference;
    typedef typename __HashTable::const_reference   const_reference;
    typedef typename __HashTable::size_type         size_type;
    //functions:
    explicit CLockHashSet(size_type elem_sz = 101, size_type lock_range = 1)
        : ht_(elem_sz, lock_range)  //elem_sz为预留的桶大小, lock_range为每把锁管辖的桶个数
    {}
    size_type Size() const{return ht_.Size();}
    bool Empty() const{return ht_.Empty();}
    void Clear(){ht_.Clear();}
    //Insert返回插入是否成功
    bool Insert(const key_type & k){
        return ht_.Insert(k);
    }
    //Find返回是否找到
    bool Find(const key_type & k) const{
        return ht_.Find(k);
    }
    //Erase返回移除的节点个数
    size_type Erase(const key_type & k){
        return ht_.Erase(k);
    }
private:
    __HashTable ht_;
};

template<
    class Key,
    class Value,
    class LockT = CRWLock,
    template<typename>class Hash = Tools::HashFn,
    template<typename>class EqualKey = std::equal_to,
    class Alloc = std::allocator<Value>
>class CLockHashMap{
    typedef CLockHashMap<Key, Value, LockT, Hash, EqualKey, Alloc>   __Myt;
public:
    typedef Key                                     key_type;
    typedef Value                                   mapped_type;
    typedef std::pair<const key_type, mapped_type>  value_type;
private:
    typedef NS_IMPL::CLockHashTable<value_type, LockT, Tools::CSelect1st<value_type>,
        EqualKey, Hash, Alloc>                  __HashTable;
public:
    typedef typename __HashTable::read_pointer      read_pointer;
    typedef typename __HashTable::write_pointer     write_pointer;
    typedef typename __HashTable::read_elem_array   read_elem_array;
    typedef typename __HashTable::write_elem_array  write_elem_array;
    typedef typename __HashTable::lock_type         lock_type;
    typedef typename __HashTable::key_equal         key_equal;
    typedef typename __HashTable::extract_key       extract_key;
    typedef typename __HashTable::hasher            hasher;
    typedef typename __HashTable::allocator_type    allocator_type;
    typedef typename __HashTable::pointer           pointer;
    typedef typename __HashTable::const_pointer     const_pointer;
    typedef typename __HashTable::reference         reference;
    typedef typename __HashTable::const_reference   const_reference;
    typedef typename __HashTable::size_type         size_type;
    //functions:
    explicit CLockHashMap(size_type elem_sz = 101, size_type lock_range = 1)
        : ht_(elem_sz, lock_range)  //elem_sz为预留的桶大小, lock_range为每把锁管辖的桶个数
    {}
    size_type Size() const{return ht_.Size();}
    bool Empty() const{return ht_.Empty();}
    void Clear(){ht_.Clear();}
    size_type BucketSize() const{return ht_.BucketSize();}
    //Insert返回插入是否成功
    //read_pointer接受返回的只读对象指针(加读锁)
    //write_pointer接受返回的可写对象指针(加写锁)
    bool Insert(const key_type & k, const mapped_type & v){
        return ht_.Insert(value_type(k, v));
    }
    bool Insert(const value_type & v){
        return ht_.Insert(v);
    }
    bool Insert(const key_type & k, const mapped_type & v, read_pointer & xp){
        return ht_.Insert(value_type(k, v), xp);
    }
    bool Insert(const value_type & v, read_pointer & xp){
        return ht_.Insert(v, xp);
    }
    bool Insert(const key_type & k, const mapped_type & v, write_pointer & xp){
        return ht_.Insert(value_type(k, v), xp);
    }
    bool Insert(const value_type & v, write_pointer & xp){
        return ht_.Insert(v, xp);
    }
    //Find返回是否找到
    bool Find(const key_type & k) const{
        return ht_.Find(k);
    }
    bool Find(const key_type & k, read_pointer & xp) const{
        return ht_.Find(k, xp);
    }
    bool Find(const key_type & k, write_pointer & xp){
        return ht_.Find(k, xp);
    }
    //遍历容器
    //Iterate返回链表是否有元素
    //wh为桶位置
    bool Iterate(size_type wh, read_elem_array & ar) const{
        return ht_.Iterate(wh, ar);
    }
    bool Iterate(size_type wh, write_elem_array & ar){
        return ht_.Iterate(wh, ar);
    }
    //Erase返回移除的节点个数
    size_type Erase(const key_type & k){
        return ht_.Erase(k);
    }
    //返回设置是否成功
    //如果k不存在, 则新插入
    bool SetValue(const key_type & k, const mapped_type & v){
        write_pointer xp;
        if(!Insert(k, v, xp) && !xp)
            return false;
        xp->second = v;
        return true;
    }
    //改变对象指针的读写锁状态
    void ReadToWrite(read_pointer & rp, write_pointer & wp) const{
        ht_.ReadToWrite(rp, wp);
    }
    void WriteToRead(write_pointer & wp, read_pointer & rp) const{
        ht_.WriteToRead(wp, rp);
    }
private:
    __HashTable ht_;
};

NS_SERVER_END

#endif
