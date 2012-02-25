#ifndef DOZERG_LOCK_HASH_TABLE_H_20070907
#define DOZERG_LOCK_HASH_TABLE_H_20070907

/*
    HashTableʵ��
        CLockHashTable      ��Ͱ���м�����hash table��ʹ�ÿ�����ʽ�����ͻ
        CMulRowHashTable    ����hash table������
    History:
        20090316    CLockHashTable����BucketSize��Iterate�������ṩ����������;��
        20111113    CLockHashTable��Hash��ģ�����ȥ��CV����
//*/

#include <vector>
#include <Tools.h>           //Tools::DestroyArray,Tools::Destroy
#include <impl/Template.h>   //COmitCV
#include <impl/LockHashTable_impl.h>

NS_IMPL_BEGIN

template<
    class Value,
    class LockT,
    class KeyOfValue,
    template<typename>class EqualKey,
    template<typename>class Hash,
    class Alloc
>class CLockHashTable
{
    //typedefs:
	typedef CLockHashTable<Value,LockT,KeyOfValue,EqualKey,Hash,Alloc>  __Myt;
public:
    typedef __hash_table_read_pointer<Value,LockT,
        KeyOfValue,EqualKey,Hash,Alloc>         read_pointer;
    typedef __hash_table_write_pointer<Value,LockT,
        KeyOfValue,EqualKey,Hash,Alloc>         write_pointer;
    typedef __hash_table_read_elem_array<Value,LockT,
        KeyOfValue,EqualKey,Hash,Alloc>         read_elem_array;
    typedef __hash_table_write_elem_array<Value,LockT,
        KeyOfValue,EqualKey,Hash,Alloc>         write_elem_array;
    typedef Value                               value_type;
    typedef LockT                               lock_type;
    typedef KeyOfValue                          extract_key;
    typedef typename extract_key::result_type   key_type;
	typedef EqualKey<key_type>                  key_equal;
    typedef Hash<typename COmitCV<key_type>::result_type>   hasher;
    typedef typename Alloc::
        template rebind<value_type>::other      allocator_type;
    typedef value_type *                        pointer;
    typedef const value_type *                  const_pointer;
    typedef value_type &                        reference;
    typedef const value_type &                  const_reference;
	typedef typename allocator_type::size_type  size_type;
private:
    typedef CLockAdapter<lock_type>         __LockAdapter;
    typedef __hashT_node_impl<value_type>   __Node;
    typedef __Node *                        __NodePtr;
    typedef lock_type *                     __LockPtr;
    typedef typename allocator_type::
        template rebind<__Node>::other      __NodeAlloc;
    typedef typename allocator_type::
        template rebind<__NodePtr>::other   __BuckAlloc;
    typedef typename allocator_type::
        template rebind<lock_type>::other   __LockAlloc;
    typedef typename allocator_type::
        template rebind<__LockPtr>::other   __LkPtrAlloc;
    //constants:
    static const size_type MIN_SIZE = 101;  //Ͱ����С����
public:
    //functions:
    //elem_sz: Ԥ����Ͱ��С
    //lock_range: ÿ������Ͻ��Ͱ����
    CLockHashTable(size_type elem_sz,size_type lock_range)
        : lock_range_(lock_range)
        , size_(0)
    {
        init(elem_sz);
    }
    ~CLockHashTable(){cleanup();}
    size_type Size() const{return size_;}
    bool Empty() const{return !Size();}
    size_type BucketSize() const{return bucket_.size();}
    //Insert���ز����Ƿ�ɹ�
    //read_pointer���ܷ��ص�ֻ������ָ��(�Ӷ���)
    //write_pointer���ܷ��صĿ�д����ָ��(��д��)
    bool Insert(const_reference v){
        key_type k = extract_key()(v);
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        writeLock(lock);
        __NodePtr p = findInBucket(k,buck_pos);
        if(!p && (p = getNode(v))){
            insertInBucket(p,buck_pos);
            ret = true;
        }
        unlock(lock);
        return ret;
    }
    bool Insert(const_reference v,read_pointer & xp){
        if(xp)
            xp.release();
        key_type k = extract_key()(v);
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        writeLock(lock);
        xp.pv_ = findInBucket(k,buck_pos);
        if(xp.pv_){  //already exists
            xp.pl_ = lock;
            write2read(lock);
        }else if((xp.pv_ = getNode(v))){
            insertInBucket(xp.pv_,buck_pos);
            xp.pl_ = lock;
            write2read(lock);
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    bool Insert(const_reference v,write_pointer & xp){
        if(xp)
            xp.release();
        key_type k = extract_key()(v);
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        writeLock(lock);
        xp.pv_ = findInBucket(k,buck_pos);
        if(xp.pv_){  //already exists
            xp.pl_ = lock;
        }else if((xp.pv_ = getNode(v))){
            insertInBucket(xp.pv_,buck_pos);
            xp.pl_ = lock;
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    //Find�����Ƿ��ҵ�
    bool Find(const key_type & k) const{
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        readLock(lock);
        ret = (findInBucket(k,buck_pos) != 0);
        unlock(lock);
        return ret;
    }
    bool Find(const key_type & k,read_pointer & xp) const{
        if(xp)
            xp.release();
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        readLock(lock);
        xp.pv_ = findInBucket(k,buck_pos);
        if(xp.pv_){  //found
            xp.pl_ = lock;
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    bool Find(const key_type & k,write_pointer & xp){
        if(xp)
            xp.release();
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        writeLock(lock);
        xp.pv_ = findInBucket(k,buck_pos);
        if(xp.pv_){  //found
            xp.pl_ = lock;
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    //Erase�����Ƴ��Ľڵ����
    size_type Erase(const key_type & k){
        size_type buck_pos = hasher()(k) % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        size_type ret = 0;
        writeLock(lock);
        ret = eraseInBucket(k,buck_pos);
        unlock(lock);
        return ret;
    }
    void Clear() __DZ_NOTHROW{
        if(!lock_range_){
            for(size_t b = 0;b < bucket_.size();++b)
                destroyBucket(bucket_[b]);
        }else{
            writeLock(lock_[0]);
            size_t k = 0;
            for(size_t b = 0,i = 0;b < bucket_.size();++b,++i){
                if(i == lock_range_){
                    unlock(lock_[k++]);
                    i -= lock_range_;
                    writeLock(lock_[k]);
                }
                destroyBucket(bucket_[b]);
            }
            unlock(lock_[k]);
        }
        size_ = 0;
    }
    //��������
    //Iterate���������Ƿ���Ԫ��
    //whΪͰλ��
    //ar�õ�������Ԫ�����飬��ʹ��ar[i],ar.size(),ar.empty()�ȷ�������Ԫ��
    bool Iterate(size_type wh,read_elem_array & ar) const{
        ar.release();
        size_type buck_pos = wh % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        readLock(lock);
        for(__NodePtr p = bucket_[buck_pos];p;p = p->next_)
            ar.ar_.push_back(p);
        if(!ar.empty()){  //not empty
            ar.pl_ = lock;
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    bool Iterate(size_type wh,write_elem_array & ar){
        ar.release();
        size_type buck_pos = wh % bucket_.size();
        __LockPtr lock = locker(buck_pos);
        bool ret = false;
        writeLock(lock);
        for(__NodePtr p = bucket_[buck_pos];p;p = p->next_)
            ar.ar_.push_back(p);
        if(!ar.empty()){  //not empty
            ar.pl_ = lock;
            ret = true;
        }else
            unlock(lock);
        return ret; //user will unlock lock when call xp.release() or destroy xp
    }
    //�ı����ָ��Ķ�д��״̬
    //�Ѷ���rp���д��wp��rp�ᱻrelease
    void ReadToWrite(read_pointer & rp,write_pointer & wp) const{
        read2write(rp.pl_);
        wp.pl_ = rp.pl_;
        wp.pv_ = rp.pv_;
        rp.pl_ = 0;
        rp.pv_ = 0;
    }
    //��д��wp��ɶ���rp��wp�ᱻrelease
    void WriteToRead(write_pointer & wp,read_pointer & rp) const{
        write2read(wp.pl_);
        rp.pl_ = wp.pl_;
        rp.pv_ = wp.pv_;
        wp.pl_ = 0;
        wp.pv_ = 0;
    }
private:
    size_type eraseInBucket(const key_type & k,size_type buck_pos){
        key_equal equaler;
        extract_key extractor;
        size_type ret = 0;
        __NodePtr p = bucket_[buck_pos], n = (p ? p->next_ : 0);
        while(n){
            if(equaler(extractor(n->data_), k)){
                p->next_ = n->next_;
                putNode(n);
                ++ret;
            }else
                p = n;
            n = p->next_;
        }
        p = bucket_[buck_pos];
        if(p && equaler(extractor(p->data_),k)){
            bucket_[buck_pos] = p->next_;
            putNode(p);
            ++ret;
        }
        size_ -= ret;
        return ret;
    }
    void insertInBucket(__NodePtr p,size_type buck_pos){
        p->next_ = bucket_[buck_pos];
        bucket_[buck_pos] = p;
        ++size_;
    }
    __NodePtr findInBucket(const key_type & k,size_type buck_pos) const{
        key_equal equaler;
        extract_key extractor;
        __NodePtr p = bucket_[buck_pos];
        for(;p;p = p->next_){
            if(equaler(extractor(p->data_),k)) //found
                return p;
        }
        return 0;
    }
    void readLock(__LockPtr pl) const{
        if(pl)
            __LockAdapter().ReadLock(*pl);
    }
    void writeLock(__LockPtr pl) const{
        if(pl)
            __LockAdapter().WriteLock(*pl);
    }
    void unlock(__LockPtr pl) const{
        if(pl)
            __LockAdapter().Unlock(*pl);
    }
    void read2write(__LockPtr pl) const{
        if(pl)
            __LockAdapter().ReadToWrite(*pl);
    }
    void write2read(__LockPtr pl) const{
        if(pl)
            __LockAdapter().WriteToRead(*pl);
    }
    __LockPtr locker(size_type buck_pos) const{
        return lock_range_ ? lock_[buck_pos / lock_range_] : 0;
    }
    void init(size_type elem_sz){
        if(elem_sz < MIN_SIZE)
            elem_sz = MIN_SIZE;
        bucket_.resize(elem_sz);
        if(lock_range_ > 0){
            elem_sz = elem_sz / lock_range_ + 1;
            lock_.resize(elem_sz);
            __LockPtr p = getLock(elem_sz);
            for(size_t i =0;i < elem_sz;++i,++p)
                lock_[i] = p;
        }
    }
    void cleanup() __DZ_NOTHROW{
        Clear();
        if(lock_range_)
            putLock(lock_[0],lock_.size());
    }
    void destroyBucket(__NodePtr & b) const __DZ_NOTHROW{
        while(b){
            __NodePtr t = b;
            b = t->next_;
            putNode(t);
        }
    }
    __LockPtr getLock(size_type sz) const{
        __LockAlloc alloc;
        __LockPtr ret = alloc.allocate(sz),p = ret,end = ret + sz;
        __DZ_TRY{
            for(;p != end;++p)
                new (p) lock_type;
        }__DZ_CATCH_ALL{
            while(p-- != ret)
                alloc.destroy(p);
            Tools::DestroyArray(ret,sz,alloc,false);
            __DZ_RETHROW;
        }
        return ret;
    }
    void putLock(__LockPtr & p,size_type sz) const __DZ_NOTHROW{
        Tools::DestroyArray(p,sz,__LockAlloc());
    }
    __NodePtr getNode(const_reference v) const{
        __NodePtr ret = 0;
        __DZ_TRY{
            ret = __NodeAlloc().allocate(1);
            allocator_type().construct(&ret->data_,v);
        }__DZ_CATCH_ALL{
            Tools::Destroy(ret,__NodeAlloc(),false);
        }
        return ret;
    }
    void putNode(__NodePtr & p) const __DZ_NOTHROW{
        Tools::Destroy(p,__NodeAlloc());
    }
    CLockHashTable(const __Myt &);
    __Myt & operator =(const __Myt &);
    //fields:
    const size_type lock_range_;
    size_type       size_;
    std::vector<__NodePtr,__BuckAlloc>  bucket_;
    std::vector<__LockPtr,__LkPtrAlloc> lock_;
};

template<
    class Value,
    class KeyOfValue,
    template<typename>class EqualKey,
    template<typename>class Hash,
    class Alloc
>class CMulRowHashTable
{
    //typedef
    typedef CMulRowHashTable<Value, KeyOfValue, EqualKey, Hash, Alloc> __Myt;
public:
    typedef Value               value_type;
    typedef value_type *        pointer;
    typedef value_type &        reference;
    typedef const value_type *  const_pointer;
    typedef const value_type &  const_reference;
    typedef Alloc               allocator_type
    typedef typename allocator_type::size_type  size_type;
    typedef KeyOfValue                          extract_key;
    typedef typename extract_key::result_type   key_type;
	typedef EqualKey<key_type>                  key_equal;
    typedef Hash<typename COmitCV<key_type>::result_type>   hasher;
    //constant
    static const size_type MAX_ROW = 100;  //�������(�����Ӱ������)
    //function
    //elemSz: Ԥ����Ԫ�ظ�����ʵ�ʴ洢��һ�㲻�ᳬ��elemSz * 95%
    //row: ������Խ��洢��Խ�ߣ�����Խ��
    //buf & sz: �����0������ָ���ڴ��ϳ�ʼ��hash table�������������ڴ�
    CMulRowHashTable(size_t elemSz, size_t row, char * buf = 0, size_t sz = 0)
        : buf_(0)
        , sz_(0)
        , mm_(false)
    {
        Init(elemSz, row, buf, sz);
    }
    //���bufΪ0�����������ڴ�
    //�����ʼ��ʧ�ܣ��򷵻�false
    bool Init(size_t elemSz, size_t row, char * buf, size_t sz){
        if(buf_)
            return false;   //re-init
        if(!elemSz || !row || elemSz <= row)
            return false;   //params error
        if(row > MAX_ROW)
            row = MAX_ROW;
        size_t col = (elemSz + row -1) / row;
        std::vector<size_t> prime_vec;
        for(size_t i = 0;i < row;++i){
            col = Tools::PrimeLess(col);
        }

        return true;
    }
private:
    void Uninit(){
    }
    //member
    char * buf_;
    size_t sz_;
    bool mm_;   //�Ƿ���������ڴ�
};

NS_IMPL_END

#endif
