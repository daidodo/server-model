#ifndef DOZERG_COMPRESSOR_ZLIB_H_20090223
#define DOZERG_COMPRESSOR_ZLIB_H_20090223

/*
    zlibÑ¹ËõËã·¨·â×°
        CCompressorZLib
//*/

#include <vector>
#include <string>
#include <common/Tools.h>

NS_EXTLIB_BEGIN
#include <zlib.h>
NS_EXTLIB_END

NS_SERVER_BEGIN

class CCompressorZLib
{
    bool    byteOrder_; //host byte order is little-endian(true) or big-endian(false)
    int     level_;     //compress level of zlib, default to -1 which means level 6
public:
    CCompressorZLib(int level = -1)
        : byteOrder_(Tools::HostByteOrder())
        , level_(level)
    {}
    void SetLevel(int lv){level_ = lv;}
    int GetLevel() const{return level_;}
    /*
    return value for Compress & Decompress
        0       success
        -1      not enough memory
        -2      not enough room in the output buffer
        -3      the level parameter is invalid
        -4      the input data was corrupted or incomplete
        -5      unkown error
    //*/
    int Compress(const __DZ_VECTOR(char) & input,__DZ_VECTOR(char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(char) & input,__DZ_VECTOR(char) & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const __DZ_VECTOR(signed char) & input,__DZ_VECTOR(signed char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(signed char) & input,__DZ_VECTOR(signed char) & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const __DZ_VECTOR(unsigned char) & input,__DZ_VECTOR(unsigned char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(unsigned char) & input,__DZ_VECTOR(unsigned char) & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const __DZ_STRING & input,__DZ_STRING & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_STRING & input,__DZ_STRING & output) const{
        return decompressTemplate(input,output);
    }
private:
    template<class Buffer>
    int compressTemplate(const Buffer & input,Buffer & output) const{
        typedef NS_EXTERN_LIB::uLongf __ZSize;
        typedef NS_EXTERN_LIB::Bytef  __ZChar;
        __ZSize in_len = __ZSize(input.size());
        __ZSize out_len = NS_EXTERN_LIB::compressBound(in_len);
        __ZSize lsz = transOrder(in_len);
        output.resize(sizeof lsz + size_t(out_len));
        memcpy(&output[0],&lsz,sizeof lsz);
        int ret = NS_EXTERN_LIB::compress2((__ZChar *)&output[sizeof lsz],&out_len,
            (const __ZChar *)&input[0],in_len,level_);
        if(Z_OK == ret){
            output.resize(size_t(out_len));
            return 0;
        }else if(Z_MEM_ERROR == ret)
            return -1;
        else if(Z_BUF_ERROR == ret)
            return -2;
        else if(Z_STREAM_ERROR == ret)
            return -3;
        return -5;
    }
    template<class Buffer>
    int decompressTemplate(const Buffer & input,Buffer & output) const{
        typedef NS_EXTERN_LIB::uLongf __ZSize;
        typedef NS_EXTERN_LIB::Bytef  __ZChar;
        if(input.size() < sizeof(__ZSize))
            return -3;
        __ZSize out_len = transOrder(*(__ZSize *)&input[0]);
        output.resize(size_t(out_len));
        int ret = NS_EXTERN_LIB::uncompress((__ZChar *)&output[0],&out_len,
            (const __ZChar *)&input[sizeof out_len],__ZSize(input.size() - sizeof(size_t)));
        if(Z_OK == ret){
            output.resize(size_t(out_len));
            return 0;
        }else if(Z_MEM_ERROR == ret)
            return -1;
        else if(Z_BUF_ERROR == ret)
            return -2;
        else if(Z_DATA_ERROR == ret)
            return -4;
        return -5;
    }
    template<typename T>
    T transOrder(T v) const{
        return byteOrder_ ? v : Tools::SwapByteOrder(v);
    }
};

NS_SERVER_END

#endif
