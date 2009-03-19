#include "ProgramVersion.h"

NS_SERVER_BEGIN

#if defined(PROGRAM_VERSION_HIGH) && defined(PROGRAM_VERSION_LOW)

CProgramVersion CProgramVersion::pv_(PROGRAM_VERSION_HIGH,PROGRAM_VERSION_LOW);

#else

CProgramVersion CProgramVersion::pv_(0,0);

#endif

NS_SERVER_END
