### already defined ###
# BASEDIR
# SRC_DIR
# SRC_SUFFIX

COMMON := $(BASEDIR)/common

    #external
EXTERN_FLAGS   :=#-pg
EXTERN__LIB    :=


#### OPTIONS ###
    #------use logger(-DLOGGER) or not
LOG            := -DLOGGER
    #------use logsys(LOGSYS=-DLOGSYS) or log4cpp(LOG4CLIB=-llog4cplus)
LOGSYS         := -DLOGSYS
#LOG4CLIB       := -llog4cplus
    #------use zlib or not
#ZIP           := -lz
    #------use openssl(-lcrypto) or not
CRYPTO         := -lcrypto
    #------use mysql(-lmysqlclient_r -lz) or not
#MYSQL          := -lmysqlclient_r -lz
    #------use epoll(-DUSEEPOLL) or poll
EPOLL          := -DUSEEPOLL
    #------use mt_alloc or not
MT_ALLOC       := -I$(COMMON)/alloc


ifneq ($(findstring release, $(MAKECMDGOALS)), release)
    DEBUG := -g
else
    RELEASE := -DNDEBUG -O2
endif

CC             := $(CXX)
INCLUDE        := -I$(COMMON) $(MT_ALLOC) -I/usr/local/ssl/include -I/usr/include/mysql
CXXFLAGS       := -Wall $(DEBUG) $(RELEASE) -D_REENTRANT $(LOG) $(LOGSYS) $(EPOLL) $(EXTERN_FLAGS) $(INCLUDE)
LIB            := -L/usr/local/ssl/lib -L/usr/local/mysql/lib -L/usr/lib -L/usr/local/lib -L$(BASEDIR)/lib -lstdc++ -lpthread -lrt $(MYSQL) $(LOG4CLIB) $(ZIP) $(CRYPTO) $(EXTERN__LIB)
CFLAGS         := $(CXXFLAGS)
ARFLAGS        := r

SRC := $(foreach dir, $(SRC_DIR), $(wildcard $(addprefix $(dir)/*., $(SRC_SUFFIX))))
OBJ := $(foreach suffix, $(SRC_SUFFIX), $(filter %.o, $(SRC:.$(suffix)=.o)))
DEP := $(OBJ:.o=.d)

CXXFLAGS += -MD
CFLAGS += -MD

