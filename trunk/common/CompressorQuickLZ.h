#ifndef DOZERG_COMPRESSOR_QUICKLZ_H_20090222
#define DOZERG_COMPRESSOR_QUICKLZ_H_20090222

/*
    QuicklzÑ¹ËõËã·¨·â×°
        CCompressorQuickLZ
//*/

#include <vector>
#include <string>
#include <common/impl/Alloc.h>

NS_SERVER_BEGIN

class CCompressorQuickLZ
{
    mutable __DZ_VECTOR(char)   workmem_;
public:
    CCompressorQuickLZ();
    /*
    return values for Compress & Decompress
        0       success
        -1      decompress input format error
    //*/
    int Compress(const __DZ_VECTOR(char) & input,__DZ_VECTOR(char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(char) & input,__DZ_VECTOR(char) & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const __DZ_VECTOR(unsigned char) & input,__DZ_VECTOR(unsigned char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(unsigned char) & input,__DZ_VECTOR(unsigned char) & output) const{
        return decompressTemplate(input,output);
    }
    int Compress(const __DZ_VECTOR(signed char) & input,__DZ_VECTOR(signed char) & output) const{
        return compressTemplate(input,output);
    }
    int Decompress(const __DZ_VECTOR(signed char) & input,__DZ_VECTOR(signed char) & output) const{
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
    int compressTemplate(const Buffer & input,Buffer & output) const;
    template<class Buffer>
    int decompressTemplate(const Buffer & input,Buffer & output) const;
};

NS_SERVER_END

#endif

/*
    NOTE:
        Undefine the STREAMING_MODE flag in quicklz.c before compiling
//*/
