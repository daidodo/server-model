#ifndef DOZERG_LOCK_HASH_SET_H_20070908
#define DOZERG_LOCK_HASH_SET_H_20070908

/*
    ��Ͱ������hash set��map, ��CLockHashTable������ʵ��
        CLockHashSet
        CLockHashMap
    History:
        20090316    CLockHashMap����BucketSize��Iterate�������ṩ����������;��
        20111113    �޸�CLockHashMap��value_type����key_type��const���Σ�����write_pointer�����޸�
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
        : ht_(elem_sz, lock_range)  //elem_szΪԤ����Ͱ��С, lock_rangeΪÿ������Ͻ��Ͱ����
    {}
    size_type Size() const{return ht_.Size();}
    bool Empty() const{return ht_.Empty();}
    void Clear(){ht_.Clear();}
    //Insert���ز����Ƿ�ɹ�
    bool Insert(const key_type & k){
        return ht_.Insert(k);
    }
    //Find�����Ƿ��ҵ�
    bool Find(const key_type & k) const{
        return ht_.Find(k);
    }
    //Erase�����Ƴ��Ľڵ����
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
        : ht_(elem_sz, lock_range)  //elem_szΪԤ����Ͱ��С, lock_rangeΪÿ������Ͻ��Ͱ����
    {}
    size_type Size() const{return ht_.Size();}
    bool Empty() const{return ht_.Empty();}
    void Clear(){ht_.Clear();}
    size_type BucketSize() const{return ht_.BucketSize();}
    //Insert���ز����Ƿ�ɹ�
    //read_pointer���ܷ��ص�ֻ������ָ��(�Ӷ���)
    //write_pointer���ܷ��صĿ�д����ָ��(��д��)
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
    //Find�����Ƿ��ҵ�
    bool Find(const key_type & k) const{
        return ht_.Find(k);
    }
    bool Find(const key_type & k, read_pointer & xp) const{
        return ht_.Find(k, xp);
    }
    bool Find(const key_type & k, write_pointer & xp){
        return ht_.Find(k, xp);
    }
    //��������
    //Iterate���������Ƿ���Ԫ��
    //whΪͰλ��
    bool Iterate(size_type wh, read_elem_array & ar) const{
        return ht_.Iterate(wh, ar);
    }
    bool Iterate(size_type wh, write_elem_array & ar){
        return ht_.Iterate(wh, ar);
    }
    //Erase�����Ƴ��Ľڵ����
    size_type Erase(const key_type & k){
        return ht_.Erase(k);
    }
    //���������Ƿ�ɹ�
    //���k������, ���²���
    bool SetValue(const key_type & k, const mapped_type & v){
        write_pointer xp;
        if(!Insert(k, v, xp) && !xp)
            return false;
        xp->second = v;
        return true;
    }
    //�ı����ָ��Ķ�д��״̬
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
