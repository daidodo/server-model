#ifndef DOZERG_FREQ_CONTROL_H_20120224
#define DOZERG_FREQ_CONTROL_H_20120224

/*
    Ƶ�ʿ���
        CFreqControl    ʹ������Ͱԭ���Ƶ�ʿ����࣬�̲߳���ȫ
//*/

#include <Tools.h>

NS_SERVER_BEGIN

struct CFreqControl
{
    CFreqControl():freq_(0){}
    //freq: Ƶ��(��/s)
    //bucketSz: ����Ͱ��С
    CFreqControl(size_t freq, size_t bucketSz)
        : freq_(0)
    {
        Init(freq, bucketSz);
    }
    //��ʼ������Ͱ�������ظ���ʼ�����޸�Ƶ�ʺ�Ͱ��С
    void Init(size_t freq, size_t bucketSz){
        if(!freq_){
            token_ = delta_ = 0;
            time_ = Tools::GetTimeUs(0);
        }
        freq_ = freq;
        buckSz_ = bucketSz;
    }
    //��������
    //nowUs: ��ǰʱ��(΢��)
    void Generate(U64 nowUs){
        if(nowUs < time_){
            time_ = nowUs;      //time jump
        }if(nowUs > time_){
            delta_ = freq_ * (nowUs - time_) + delta_;
            if(delta_ < 0){
                delta_ = 0;     //overflow
                token_ = buckSz_;
            }else{
                ssize_t tok = delta_ / 1000000 + token_;
                delta_ %= 1000000;
                if(tok < token_ || tok > buckSz_)
                    tok = buckSz_;  //overflow
                token_ = tok;
            }
        }
    }
    void Generate(){return Generate(Tools::GetTimeUs(0));}
    //��������Ƿ��㹻
    //need: ��Ҫ��������Ŀ
    bool Check(size_t need) const{return token_ >= need;}
    //��ȡ��ǰ������
    ssize_t Token() const{return token_;}
    //�۳�����
    //���������������������п۳�������������false��
    bool Get(size_t need = 1){
        if(token_ < need)
            return false;
        token_ -= need;
        return true;
    }
    //͸֧���ƣ����ܵ���token_Ϊ��
    //���need����(overflow)��������п۳�������������false
    bool Overdraft(size_t need){
        if(token_ + need < token_)
            return false;
        token -= need;
        return true;
    }
private:
    size_t freq_;
    size_t buckSz_;
    ssize_t token_;
    ssize_t delta_;
    U64 time_;      //�ϴ�generate��ʱ��(΢��)
};

NS_SERVER_END

#endif

