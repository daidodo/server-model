#include "CompressorQuickLZ.h"

NS_EXTLIB_BEGIN
#include <common/quicklz/quicklz.c>
#ifdef STREAMING_MODE
#   error STREAMING_MODE flag cannot be used in CCompressorQuickLZ
#endif
NS_EXTLIB_END

NS_SERVER_BEGIN

CCompressorQuickLZ::CCompressorQuickLZ()
    : workmem_(SCRATCH_COMPRESS)
{}

template<class Buffer>
int CCompressorQuickLZ::compressTemplate(const Buffer & input,Buffer & output) const
{
    output.resize(input.size() + 400);
    size_t out_len = NS_EXTERN_LIB::qlz_compress(&input[0],(char *)&output[0],input.size(),&workmem_[0]);
    output.resize(out_len);
    return 0;
}

template<class Buffer>
int CCompressorQuickLZ::decompressTemplate(const Buffer & input,Buffer & output) const
{
    if(input.size() < 9)
        return -1;
    size_t out_len = NS_EXTERN_LIB::qlz_size_decompressed((const char *)&input[0]);
    output.resize(out_len);
    out_len = NS_EXTERN_LIB::qlz_decompress((const char *)&input[0],&output[0],&workmem_[0]);
    output.resize(out_len);
    return 0;
}

NS_SERVER_END
