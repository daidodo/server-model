#ifndef DOZERG_FREQ_CONTROL_H_20120224
#define DOZERG_FREQ_CONTROL_H_20120224

/*
    频率控制
        CFreqControl    使用令牌桶原理的频率控制类，线程不安全
//*/

#include <Tools.h>

NS_SERVER_BEGIN

struct CFreqControl
{
    CFreqControl():freq_(0){}
    //freq: 频率(次/s)
    //bucketSz: 令牌桶大小
    CFreqControl(size_t freq, size_t bucketSz)
        : freq_(0)
    {
        Init(freq, bucketSz);
    }
    //初始化令牌桶，可以重复初始化，修改频率和桶大小
    void Init(size_t freq, size_t bucketSz){
        if(!freq_){
            token_ = delta_ = 0;
            time_ = Tools::GetTimeUs(0);
        }
        freq_ = freq;
        buckSz_ = bucketSz;
    }
    //生产令牌
    //nowUs: 当前时间(微秒)
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
    //检查令牌是否足够
    //need: 需要的令牌数目
    bool Check(size_t need) const{return token_ >= need;}
    //获取当前令牌数
    ssize_t Token() const{return token_;}
    //扣除令牌
    //如果令牌数不够，不会进行扣除操作，并返回false，
    bool Get(size_t need = 1){
        if(token_ < need)
            return false;
        token_ -= need;
        return true;
    }
    //透支令牌，可能导致token_为负
    //如果need过大(overflow)，不会进行扣除操作，并返回false
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
    U64 time_;      //上次generate的时间(微秒)
};

NS_SERVER_END

#endif

