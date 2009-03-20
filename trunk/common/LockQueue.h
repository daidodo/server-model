#ifndef DOZERG_LOCKED_QUEUE_H_20070901
#define DOZERG_LOCKED_QUEUE_H_20070901

/*
    ����������FIFO��Ϣ����,�ʺ��̼߳䴫������
        CLockQueue
    History
        20070925    ��pthread_cond_t��pthread_mutex_t�滻��CCondMutex
        20071025    ����capacity_,����CCondMutex�ֳɺ�CMutex��CCondition
        20080128    ����PushFront(),��Ԫ�طŵ�����ǰ��
        20080203    ����Mutex(),ͳһ�ӿ�
        20080903    ����PopAll(),����Ƶ��Pop()
        20080911    ����PushAll(),����Ƶ��Push()
        20080912    ����_append(),������PushAll()�����ͽ��нӿ�ͳһ
        20080920    ����lock_type��adapter_type��guard_type���޸�GetLock()��Lock()��Unlock()
//*/

#include <common/Mutex.h>
#include <common/List.h>
#include <common/SingleList.h>

NS_SERVER_BEGIN

template<class T,class Container = CSingleList<T> >
class CLockQueue : public Container
{
    typedef CLockQueue<T,Container>             __Myt;
    typedef Container                           __MyBase;
public:
    typedef Container                           container_type;
    typedef typename __MyBase::value_type       value_type;
    typedef typename __MyBase::size_type        size_type;
    typedef typename __MyBase::reference        reference;
    typedef typename __MyBase::const_reference  const_reference;
    typedef typename __MyBase::const_iterator   const_iterator;
    typedef typename __MyBase::iterator         iterator;
    typedef CMutex                              lock_type;
    typedef CLockAdapter<lock_type>             adapter_type;
    typedef CGuard<lock_type>                   guard_type;
private:
    static const size_t CAPACITY_DEFAULT = 10000;   //Ĭ������
    template<class E,class A>
    static void _append(CSingleList<E,A> & to,CSingleList<E,A> & from){
        to.append(from);
    }
    template<class E,class A>
    static void _append(CList<E,A> & to,CList<E,A> & from){
        to.splice(to.end(),from);
    }
public:
    explicit CLockQueue(size_t capacity = CAPACITY_DEFAULT)
        : capacity_(capacity)
        , top_size_(0)
    {}
    explicit CLockQueue(const __MyBase & con,size_t capacity = CAPACITY_DEFAULT)
        : __MyBase(con)
        , capacity_(capacity)
        , top_size_(con.size())
    {}
    size_type Capacity() const{return capacity_;}
    void Capacity(size_type c){capacity_ = c;}
    lock_type & GetLock(){return lock_;}
    void Lock(bool bWrite = true){
        bWrite ? adapter_type().WriteLock(lock_) :
            adapter_type().ReadLock(lock_);
    }
    void Unlock(){
        adapter_type().Unlock(lock_);
    }
    bool Empty() const{
        guard_type guard(lock_);
        return __MyBase::empty();
    }
    size_type Size() const{
        guard_type guard(lock_);
        return __MyBase::size();
    }
    //timeMs < 0,����ʽ;timeMs >= 0,�ȴ�timeMs����
    bool Push(const_reference v,S32 timeMs = -1){
        guard_type guard(lock_);
        const int sig = waitNotFull(timeMs);
        if(sig < 0)
            return false;
        __MyBase::push_back(v);
        const size_t sz = __MyBase::size();
        if(sz > top_size_)  //for statistic
            top_size_ = sz;
        if(sig)
            not_empty_.Broadcast();
        return true;
    }
    //timeMs < 0,����ʽ;timeMs >= 0,�ȴ�timeMs����
    bool PushFront(const_reference v,S32 timeMs = -1){
        guard_type guard(lock_);
        const int sig = waitNotFull(timeMs);
        if(sig < 0)
            return false;
        __MyBase::push_front(v);
        const size_t sz = __MyBase::size();
        if(sz > top_size_)  //for statistic
            top_size_ = sz;
        if(sig)
            not_empty_.Broadcast();
        return true;
    }
    //��con������Ԫ�ؼӵ�����β
    //timeMs < 0,����ʽ;timeMs >= 0,�ȴ�timeMs����
    //ֻ��������CLockQueue::_append���������ͣ�����ʹ�ô˲���
    bool PushAll(container_type & con,S32 timeMs = -1){
        if(con.empty())
            return true;
        guard_type guard(lock_);
        const int sig = waitNotFull(timeMs);
        if(sig < 0)
            return false;
        _append(*this,con);
        const size_t sz = __MyBase::size();
        if(sz > top_size_)  //for statistic
            top_size_ = sz;
        if(sig)
            not_empty_.Broadcast();
        return true;
    }
    //timeMs < 0,����ʽ;timeMs >= 0,�ȴ�timeMs����
    bool Pop(reference v,S32 timeMs = -1){
        guard_type guard(lock_);
        const int sig = waitNotEmpty(timeMs);
        if(sig < 0)
            return false;
        v = __MyBase::front();
        __MyBase::pop_front();
        if(sig)
            not_full_.Broadcast();
        return true;
    }
    //�Ѷ��е�����Ԫ��ת�Ƶ�con��
    //timeMs < 0,����ʽ;timeMs >= 0,�ȴ�timeMs����
    bool PopAll(container_type & con,S32 timeMs = -1){
        con.clear();
        guard_type guard(lock_);
        const int sig = waitNotEmpty(timeMs);
        if(sig < 0)
            return false;
        __MyBase::swap(con);
        if(sig)
            not_full_.Broadcast();
        return true;
    }
    size_t TopSize() const{    //���г��ȵķ�ֵ
        guard_type guard(lock_);
        return top_size_;
    }
    size_t ResetTopSize(){
        guard_type guard(lock_);
        size_t ret = top_size_;
        top_size_ = __MyBase::size();
        return ret;
    }
private:
    /*
    Return Value:
        -1  failed
        0   not full
        1   full
    //*/
    int waitNotEmpty(S32 timeMs){
        while(__MyBase::empty()){
            if(timeMs < 0){
                not_empty_.Wait(lock_);
            }else if(!timeMs || !not_empty_.TimeWait(lock_,timeMs))
                return -1;
        }
        return (__MyBase::size() >= capacity_ ? 1 : 0);
    }
    /*
    Return Value:
        -1  failed
        0   not empty
        1   empty
    //*/
    int waitNotFull(S32 timeMs){
        while(__MyBase::size() >= capacity_){
            if(timeMs < 0){
                not_full_.Wait(lock_);
            }else if(!timeMs || !not_full_.TimeWait(lock_,timeMs))
                return -1;
        }
        return (__MyBase::empty() ? 1 : 0);
    }
    CLockQueue(const __Myt &);
    __Myt & operator =(const __Myt &);
    size_type   capacity_;      //������󳤶�,�ﵽcapacity_��Push������
    size_type   top_size_;      //__MyBase::size()�ķ�ֵ,ͳ����
    lock_type   lock_;
    CCondition  not_empty_,not_full_;
};

NS_SERVER_END

#endif
