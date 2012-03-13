#ifndef DOZERG_COMPRESSOR_LZO_H_20080221
#define DOZERG_COMPRESSOR_LZO_H_20080221

/*
    LZOÑ¹ËõËã·¨·â×°
        CCompressorLZO
//*/

#include <vector>
#include <string>
#include <cstring>          //memcpy
#include <Tools.h>   //Tools::SwapByteOrder, Tools::HostByteOrder
#include <lzo/minilzo.h>

NS_SERVER_BEGIN

class CCompressorLZO
{
    bool    byteOrder_; //host byte order is little-endian(true) or big-endian(false)
    mutable std::vector<unsigned char>  workmem_;   //Memory required for the wrkmem parameter
public:
    CCompressorLZO()
        : byteOrder_(Tools::HostByteOrder())
    {
        if(lzo_init() == LZO_E_OK)
            workmem_.resize(LZO1X_1_MEM_COMPRESS);
    }
    /*
    return value for Compress & Decompress
        0       success
        -1      not initialized
        -2      compress error
        -3      decompress input format error
        -4      decompress error
    //*/
    int Compress(const std::vector<char> & input,std::vector<char> & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const std::vector<char> & input,std::vector<char> & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const std::vector<signed char> & input,std::vector<signed char> & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const std::vector<signed char> & input,std::vector<signed char> & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const std::vector<unsigned char> & input,std::vector<unsigned char> & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const std::vector<unsigned char> & input,std::vector<unsigned char> & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const std::string & input,std::string & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const std::string & input,std::string & output) const{
        return decompressTemplate(input,output);
    }
private:
    template<class Buffer>
    int compressTemplate(const Buffer & input,Buffer & output) const{
        if(workmem_.empty())
            return -1;
        lzo_uint out_len = lzo_uint(outLength(input.size()));
        output.resize(sizeof(size_t) + size_t(out_len));
        size_t lsz = transOrder(input.size());
        memcpy(&output[0],&lsz,sizeof lsz);
        lzo_uint in_len = lzo_uint(input.size());
        if(LZO_E_OK != lzo1x_1_compress((const unsigned char *)&input[0],in_len,
            (unsigned char *)&output[sizeof(size_t)],&out_len,&workmem_[0]))
        {
            return -2;
        }
        output.resize(sizeof(size_t) + out_len);
        return 0;
    }
    template<class Buffer>
    int decompressTemplate(const Buffer & input,Buffer & output) const{
        if(input.size() < sizeof(size_t))
            return -3;
        lzo_uint out_len = lzo_uint(transOrder(*(size_t *)&input[0]));
        output.resize(out_len);
        if(LZO_E_OK != lzo1x_decompress((const unsigned char *)&input[sizeof(size_t)],
            lzo_uint(input.size() - sizeof(size_t)),(unsigned char *)&output[0],&out_len,0))
        {
            return -4;
        }
        output.resize(out_len);
        return 0;
    }
    size_t outLength(size_t inLen) const{
        return inLen + (inLen >> 4) + 67;
    }
    size_t transOrder(size_t v) const{
        return byteOrder_ ? v : Tools::SwapByteOrder(v);
    }
};

NS_SERVER_END

#endif
