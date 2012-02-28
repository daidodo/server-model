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
#include <numeric>              //std::accumulate
#include <Tools.h>              //Tools::DestroyArray,Tools::Destroy
#include <Template.h>           //COmitCV
#include <impl/HashTable_impl.h>

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
            for(size_type b = 0;b < bucket_.size();++b)
                destroyBucket(bucket_[b]);
        }else{
            writeLock(lock_[0]);
            size_type k = 0;
            for(size_type b = 0,i = 0;b < bucket_.size();++b,++i){
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
            for(size_type i =0;i < elem_sz;++i,++p)
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
    typedef Alloc               allocator_type;
    typedef typename allocator_type::size_type  size_type;
    typedef KeyOfValue                          extract_key;
    typedef typename extract_key::result_type   key_type;
	typedef EqualKey<key_type>                  key_equal;
    typedef Hash<typename COmitCV<key_type>::result_type>   hasher;
private:
    typedef __mr_hash_table_head    __Head;
    typedef typename allocator_type::template rebind<char>::other __BufAlloc;
public:
    //constant
    static const size_type MAX_ROW = 100;   //�������(�����Ӱ������)
    static const int VERSION = 1;           //�ڴ沼�ְ汾��
    //function
    CMulRowHashTable()
        : head_(0)
        , body_(0)
        , sz_(0)
        , mm_(false)
    {}
    ~CMulRowHashTable(){uninit();}
    //�����Ƿ��ʼ���ɹ�
    bool IsValid() const{return head_ != 0;}
    //������hash table
    //row: ������Խ��洢��Խ�ߣ�����Խ��
    //col: ÿ�е�Ԫ�ظ���������
    //buf & sz: �����0������ָ���ڴ��ϳ�ʼ��hash table�������������ڴ�
    bool Create(size_type row, size_type col, char * buf = 0, size_type sz = 0){
        return create(row, col, buf, sz);
    }
    //��ȡ����hash table
    //buf & sz: ����hash table���ڴ�
    //row & col: ���������������Ϊ0����δָ��
    bool Attach(char * buf, size_type sz, size_type row = 0, size_type col = 0){
        return attach(buf, sz, row, col);
    }
    //�Ƿ�Ϊ��
    bool Empty() const{return Size() == 0;}
    //�����ɵ����Ԫ�ظ���
    size_type MaxSize() const{return head_->Sum();}
    //��ռ�õ�Ԫ�ظ���
    size_type Size() const{return head_->Used();}
    //����
    size_type Row() const{return head_->Row();}
    //����
    size_type Col() const{return head_->Col();}
    //����Ԫ��
    //return:
    //      <true, pointer> - �´����ڵ�ɹ���pointerΪ�ڵ�ָ��
    //      <false, pointer> - �ҵ����нڵ㣬pointerΪ�ڵ�ָ��
    //      <false, 0> - �޷������½ڵ�
    std::pair<bool, pointer> Insert(const_reference v){
        bool succ = false;
        pointer p = 0;
        if(IsValid()){
            if((succ = findKey(extract_key()(v), p, true))){
                assert(p);
                *p = v;
                head_->Used(1);
            }
        }
        return std::make_pair(succ, p);
    }
    //����Ԫ��
    //return:
    //      <true, pointer> - �ҵ�Ԫ�أ�pointerΪ�ڵ�ָ��
    //      <false, 0> - δ�ҵ�Ԫ��
    std::pair<bool, pointer> Find(const_reference v){
        bool succ = false;
        pointer p = 0;
        if(IsValid())
            succ = findKey(extract_key()(v), p, false);
        return std::make_pair(succ, p);
    }
    std::pair<bool, const_pointer> Find(const_reference v) const{
        bool succ = false;
        const_pointer p = 0;
        if(IsValid())
            succ = findKey(extract_key()(v), p, false);
        return std::make_pair(succ, p);
    }
    //ɾ��Ԫ��
    //�����Ƿ�����ɾ���˽ڵ�
    bool Erase(const_reference v){
        pointer p = 0;
        if(IsValid()){
            if(findKey(extract_key()(v), p, false)){
                assert(p);
                *p = value_type();
                head_->Used(-1);
                return true;
            }
        }
        return false;
    }
    bool Erase(pointer p){
        if(!IsValid())
            return false;
        if(p < body_)
            return false;
        size_type sz = p - body_;
        if(sz >= head_->Sum())
            return false;
        if(p != body_ + sz)
            return false;
        if(key_equal()(extract_key()(*p), key_type()))
            return false;
        *p = value_type();
        head_->Used(-1);
        return true;
    }
    //����Ԫ��
    //pos: [0, MaxSize())��Χ�ڵ�ֵ
    //return: �ڵ�ָ�룻����ڵ�δʹ�ã�����0
    pointer Iterate(size_type pos){
        pointer p = 0;
        if(IsValid() && pos < head_->Sum())
            p = body_ + pos;
        return p;
    }
    const_pointer Iterate(size_type pos) const{
        const_pointer p = 0;
        if(IsValid() && pos < head_->Sum())
            p = body_ + pos;
        return p;
    }
private:
    //���δ�ҵ�k����empty=true����p��Ҫ����δʹ�ýڵ�
    bool findKey(const key_type & k, pointer & p, bool empty){
        assert(IsValid());
        pointer e = 0;  //empty node
        size_type hash = hasher()(k);
        for(size_type sum = 0, r = 0;r < head_->Row();++r){
            const size_type prime = head_->Prime(r);
            const pointer v = body_ + (hash % prime + sum);
            const key_type vk = extract_key()(*v);
            if(key_equal()(k, vk)){
                p = v;
                return true;
            }
            if(empty && !e && key_equal()(vk, key_type()))
                e = v;
            sum += prime;
        }
        p = e;
        return false;
    }
    bool findKey(const key_type & k, const_pointer & p) const{
        assert(IsValid());
        size_type hash = hasher()(k);
        for(size_type sum = 0, r = 0;r < head_->Row();++r){
            const size_type prime = head_->Prime(r);
            const const_pointer v = body_ + (hash % prime + sum);
            const key_type vk = extract_key()(*v);
            if(key_equal()(k, vk)){
                p = v;
                return true;
            }
            sum += prime;
        }
        p = 0;
        return false;
    }
    bool create(size_type row, size_type col, char * buf, size_type sz){
        //check
        if(head_)
            return false;   //re-create
        if(!col || !row)
            return false;   //params error
        if(row > MAX_ROW)
            row = MAX_ROW;
        //primes
        size_type p = col;
        std::vector<size_type> vec;
        for(size_type i = 0;i < row;++i, --p){
            p = Tools::PrimeLess(p);
            if(!p)
                return false;   //params error
            vec.push_back(p);
        }
        //size
        size_type sum = std::accumulate(vec.begin(), vec.end(), 0);
        size_type hs = headSz(row);    //head size
        size_type bs = bodySz(sum);    //body size
        //alloc
        bool mm = false;
        if(!buf){
            sz = hs + bs;
            buf = __BufAlloc().allocate(sz);
            mm = true;
        }
        //init
        __Head * head = 0;
        pointer body = 0;
        if(sz < hs + bs)
            return false;   //buf not enough
        head = reinterpret_cast<__Head *>(buf);
        body = reinterpret_cast<pointer>(buf + hs);
        assert(head && body);
        head->Init(VERSION, row, col, vec);
        std::uninitialized_fill_n(body, sum, value_type());
        //set
        head_ = head;
        body_ = body;
        sz_ = sz;
        mm_ = mm;
        return true;
    }
    bool attach(char * buf, size_type sz, size_type row, size_type col){
        //check
        if(head_)
            return false;   //re-attach
        if(!buf || !sz)
            return false;   //params error;
        if(row > MAX_ROW)
            row = MAX_ROW;
        //head
        __Head * head = reinterpret_cast<__Head *>(buf);
        assert(head);
        if(!head->Check(VERSION, row, col))
            return false;   //head error
        //size
        size_type hs = headSz(head->Row());
        if(sz <= hs)
            return false;   //sz not enough
        size_type bs = bodySz(head->Sum());
        if(sz < hs + bs)
            return false;   //sz not enough
        //set
        head_ = head;
        body_ = reinterpret_cast<pointer>(buf + hs);
        sz_ = sz;
        mm_ = false;
        return true;
    }
    void uninit(){
        if(!IsValid() || !mm_)
            return;
        for(size_type i = 0;i < head_->Sum();++i)
            body_[i].~value_type();
        char * buf = reinterpret_cast<char *>(head_);
        __BufAlloc().deallocate(buf, sz_);
    }
    size_type headSz(size_type row) const{
        return sizeof(__Head) + sizeof(U64) * row;
    }
    size_type bodySz(size_type sum) const{
        return sizeof(value_type) * sum;
    }
    //member
    __Head * head_;
    pointer body_;
    size_type sz_;
    bool mm_;   //�Ƿ���������ڴ�
};

NS_IMPL_END

#endif
