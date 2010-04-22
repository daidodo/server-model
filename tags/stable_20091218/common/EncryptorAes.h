#ifndef DOZERG_ENCRYPTOR_AES_H_20080306
#define DOZERG_ENCRYPTOR_AES_H_20080306

/*
    Aes�����㷨��װ
    Aes�����û�ָ����keystr_�ַ���(char[16])���ڴ˻����ϸ��ݲ�ͬ����Կǿ��keyInten_��
    ���ɲ�ͬ�ļ�����Կ�ͽ�����Կ(AES_KEY)���ֱ����ڼ��ܺͽ���
    keystr_�����û�ָ����key����md5�㷨�õ�
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
    static const size_t KEY_LEN = 16;   //Aesֻʹ��keystr_��ǰ16�ֽ�
public:
    enum EKeyIntensity{     //Aes֧�ֵ�3����Կǿ��
        L128 = 128,
        L192 = 192,
        L256 = 256
    };
    //�����û���Կ�ͼ���ǿ��
    //keyΪ�û�֪���ļ��ܽ��ܵ���Կ
    //keylenΪkey�ĳ���
    //intensityΪ����ǿ��
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
    //����/����input�д�fromƫ�ƿ�ʼ������,�������output��
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
    EKeyIntensity   keyInten_;  //��Կǿ��
    AES_KEY         enKey_;     //������Կ
    AES_KEY         deKey_;     //������Կ
};

NS_SERVER_END

#endif
