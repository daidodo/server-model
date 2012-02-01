#include <sstream>

#include "RecvHelper.h"

NS_SERVER_BEGIN

std::string CRecvHelper::ToString() const{
    std::ostringstream oss;
    oss<<"{initSz_="<<initSz_
        <<"}";
    return oss.str();
}

NS_SERVER_END
