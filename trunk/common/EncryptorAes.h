#ifndef DOZERG_ENCRYPTOR_AES_H_20080306
#define DOZERG_ENCRYPTOR_AES_H_20080306

/*
    Aes加密算法封装
    Aes接受用户指定的keystr_字符串(char[16])，在此基础上根据不同的密钥强度keyInten_，
    生成不同的加密密钥和解密密钥(AES_KEY)，分别用于加密和解密
    keystr_根据用户指定的key经过md5算法得到
//*/

#include <string>
#include <vector>
#include <cstring>              //memset
#include <openssl/aes.h>
#include <openssl/md5.h>
#include <common/impl/Alloc.h>

NS_SERVER_BEGIN

class CEncryptorAes
{
    static const size_t KEY_LEN = 16;   //Aes只使用keystr_的前16字节
public:
    enum EKeyIntensity{     //Aes支持的3种密钥强度
        L128 = 128,
        L192 = 192,
        L256 = 256
    };
    //设置用户密钥和加密强度
    //key为用户知道的加密解密的密钥
    //keylen为key的长度
    //intensity为加密强度
    void SetKey(__DZ_STRING key,EKeyIntensity intensity = L128){
        SetKey((const U8 *)key.c_str(),key.length(),intensity);
    }
    void SetKey(const char * key,size_t keylen,EKeyIntensity intensity = L128){
        SetKey((const U8 *)key,keylen,intensity);
    }
    void SetKey(const S8 * key,size_t keylen,EKeyIntensity intensity = L128){
        SetKey((const U8 *)key,keylen,intensity);
    }
    void SetKey(const U8 * key,size_t keylen,EKeyIntensity intensity = L128){
        keyInten_ = intensity;
        memset(keystr_,0,KEY_LEN);
        MD5(key,keylen,keystr_);
        AES_set_encrypt_key(keystr_,keyInten_,&enKey_);
        AES_set_decrypt_key(keystr_,keyInten_,&deKey_);
    }
    /*
    return value for Encrypt & Decrypt
        0       success
        -1      param error
        -2      decrypt data format error
    //*/
    //加密/解密input中从from偏移开始的数据,结果放入output中
    int Encrypt(const __DZ_VECTOR(char) & input,size_t from,__DZ_VECTOR(char) & output) const{
        return encryptTemplate(input,from,output);
    }
    int Decrypt(const __DZ_VECTOR(char) & input,size_t from,__DZ_VECTOR(char) & output) const{
        return decryptTemplate(input,from,output);
    }
    int Encrypt(const __DZ_VECTOR(signed char) & input,size_t from,__DZ_VECTOR(signed char) & output) const{
        return encryptTemplate(input,from,output);
    }
    int Decrypt(const __DZ_VECTOR(signed char) & input,size_t from,__DZ_VECTOR(signed char) & output) const{
        return decryptTemplate(input,from,output);
    }
    int Encrypt(const __DZ_VECTOR(unsigned char) & input,size_t from,__DZ_VECTOR(unsigned char) & output) const{
        return encryptTemplate(input,from,output);
    }
    int Decrypt(const __DZ_VECTOR(unsigned char) & input,size_t from,__DZ_VECTOR(unsigned char) & output) const{
        return decryptTemplate(input,from,output);
    }
    int Encrypt(const __DZ_STRING & input,size_t from,__DZ_STRING & output) const{
        return encryptTemplate(input,from,output);
    }
    int Decrypt(const __DZ_STRING & input,size_t from,__DZ_STRING & output) const{
        return decryptTemplate(input,from,output);
    }
private:
    template<class Buffer>
    int encryptTemplate(const Buffer & input,size_t from,Buffer & output) const{
        size_t inlen = input.size();
        if(inlen < from)
            return -1;
        inlen -= from;
        output.resize(from + (inlen / AES_BLOCK_SIZE + 1) * AES_BLOCK_SIZE);
        std::copy(input.begin(),input.begin() + from,output.begin());
        for(;inlen >= AES_BLOCK_SIZE;inlen -= AES_BLOCK_SIZE,from += AES_BLOCK_SIZE)
            AES_encrypt((const U8 *)&input[from],(U8 *)&output[from],&enKey_);
        U8 tmp[AES_BLOCK_SIZE];
        memset(tmp,AES_BLOCK_SIZE - inlen,AES_BLOCK_SIZE);
        memcpy(tmp,&input[from],inlen);
        AES_encrypt(tmp,(U8 *)&output[from],&enKey_);
        return 0;
    }
    template<class Buffer>
    int decryptTemplate(const Buffer & input,size_t from,Buffer & output) const{
        size_t inlen = input.size();
        if(inlen < from || (inlen - from) % AES_BLOCK_SIZE != 0)
            return -1;
        output.resize(inlen);
        std::copy(input.begin(),input.begin() + from,output.begin());
        for(inlen -= from;inlen;inlen -= AES_BLOCK_SIZE,from += AES_BLOCK_SIZE)
            AES_decrypt((const U8 *)&input[from],(U8 *)&output[from],&deKey_);
        inlen = *output.rbegin();
        if(inlen == 0 || inlen > AES_BLOCK_SIZE)
            return -2;
        output.resize(output.size() - inlen);
        return 0;
    }
    U8              keystr_[KEY_LEN];
    EKeyIntensity   keyInten_;  //密钥强度
    AES_KEY         enKey_;     //加密密钥
    AES_KEY         deKey_;     //解密密钥
};

NS_SERVER_END

#endif
