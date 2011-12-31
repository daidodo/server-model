#ifndef DOZERG_COMPRESSOR_QUICKLZ_H_20090222
#define DOZERG_COMPRESSOR_QUICKLZ_H_20090222

/*
    QuicklzÑ¹ËõËã·¨·â×°
        CCompressorQuickLZ
//*/

#include <vector>
#include <string>
#include <impl/Config.h>

NS_SERVER_BEGIN

class CCompressorQuickLZ
{
    mutable std::vector<char>   workmem_;
public:
    CCompressorQuickLZ();
    /*
    return values for Compress & Decompress
        0       success
        -1      decompress input format error
    //*/
    int Compress(const std::vector<char> & input,std::vector<char> & output) const;
    int Decompress(const std::vector<char> & input,std::vector<char> & output) const;
    int Compress(const std::vector<unsigned char> & input,std::vector<unsigned char> & output) const;
    int Decompress(const std::vector<unsigned char> & input,std::vector<unsigned char> & output) const;
    int Compress(const std::vector<signed char> & input,std::vector<signed char> & output) const;
    int Decompress(const std::vector<signed char> & input,std::vector<signed char> & output) const;
    int Compress(const std::string & input,std::string & output) const;
    int Decompress(const std::string & input,std::string & output) const;
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
