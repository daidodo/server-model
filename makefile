COMMONDIR      :=common/ common/lzo/
SERVERDIR      :=server/ server/frame/

BINDIR         :=bin/
SERVER_TARGET  :=Server.out
TESTDIR        :=test/
TEST_TARGET    :=test.out
SRC_SUFFIX     :=cpp c

    #external
EXTERN_FLAGS   :=#-O3
EXTERN__LIB    :=

####以下是可选的编译参数
    #------use logger(-DLOGGER) or not
LOG            :=-DLOGGER
    #------use logsys(LOGSYS=-DLOGSYS) or log4cpp(LOG4CLIB=-llog4cplus)
LOGSYS         :=-DLOGSYS
#LOG4CLIB       :=-llog4cplus
    #------debug mode or not(-DNDEBUG)
#RELEASE        :=-DNDEBUG
    #------use zlib or not
#ZIP           :=-lz
    #------use openssl(-lcrypto) or not
CRYPTO         :=-lcrypto
    #------use mysql(-lmysqlclient_r -lz) or not
#MYSQL          :=-lmysqlclient_r -lz
    #------use epoll(-DUSEEPOLL) or poll
EPOLL          :=-DUSEEPOLL
  #program versions
DATE           :=$(shell date +"%Y%m%d")
TIME           :=$(shell date +"%H%M%S")
HVERSION       :=-DPROGRAM_VERSION_HIGH=$(DATE)
LVERSION       :=-DPROGRAM_VERSION_LOW=1$(TIME)-1000000
PROGRAMVERSION :=$(SERVERDIR)ProgramVersion

CC             :=$(CXX)
INCLUDE        :=-I./ -I/usr/local/mysql/include/
CXXFLAGS       :=-Wall -g $(INCLUDE) $(LOG) $(LOGSYS) $(RELEASE) $(EPOLL) $(EXTERN_FLAGS)
LIB            :=-L/usr/local/mysql/lib -L/usr/lib -L/usr/local/lib -L./lib -lstdc++ -lpthread -lrt $(MYSQL) $(LOG4CLIB) $(ZIP) $(CRYPTO) $(EXTERN__LIB)
CFLAGS         :=$(CXXFLAGS)

####以下是一些有用的函数

#下面用$(1)代替$@的原因是，在规则依赖里，$@不起作用
#由每个TEST_TARGET文件名得到所需的.o文件列表
TEST_OBJS=$(filter $(dir $(1))%.o,$(TESTOBJ))

#创建每个TEST_TARGET的规则
define BUILD_TEST
$(1) : $$(COMMONOBJ) $$(call TEST_OBJS,$(1))
	$$(CXX) $$^ $$(CXXFLAGS) -o $$@ $$(LIB)
endef

#得到所有SRC_SUFFIX后缀的源代码文件
GETSRC=$(foreach dir,$(1),$(wildcard $(addprefix $(dir)*.,$(SRC_SUFFIX))))

#得到.o文件
GETOBJ=$(foreach suffix,$(SRC_SUFFIX),$(filter %.o,$($(1):.$(suffix)=.o)))

####以下是工程创建规则，请勿盲目修改

TESTSRC:=$(call GETSRC,$(TESTDIR))
SERVERSRC:=$(call GETSRC,$(SERVERDIR))
COMMONSRC:=$(call GETSRC,$(COMMONDIR))

TESTOBJ:=$(call GETOBJ,TESTSRC)
SERVEROBJ:=$(call GETOBJ,SERVERSRC)
COMMONOBJ:=$(call GETOBJ,COMMONSRC)

TESTDEP:=$(TESTOBJ:.o=.d)
SERVERDEP:=$(SERVEROBJ:.o=.d)
COMMONDEP:=$(COMMONOBJ:.o=.d)

CXXFLAGS+=-MD

ULTIMATE_TARGET:=$(SERVER_TARGET).$(DATE).$(TIME)
TEST_TARGET_LIST:=$(join $(TESTDIR),$(TEST_TARGET))

all: $(BINDIR)$(SERVER_TARGET)

link: all
	@echo $(SERVER_TARGET) -\> $(ULTIMATE_TARGET)
	@mv $(BINDIR)$(SERVER_TARGET) $(BINDIR)$(ULTIMATE_TARGET)
	@ln -sf $(ULTIMATE_TARGET) $(BINDIR)$(SERVER_TARGET)

$(BINDIR)$(SERVER_TARGET) : $(COMMONOBJ) $(SERVEROBJ)
	$(CXX) $^ $(CXXFLAGS) -o $@ $(LIB)

test: $(TEST_TARGET_LIST)

$(foreach target,$(TEST_TARGET_LIST),$(eval $(call BUILD_TEST,$(target))))

$(PROGRAMVERSION).o : force
	$(CXX) $(CXXFLAGS) -c -o $@ $*.cpp $(HVERSION) $(LVERSION)

force : ;

commonobj : $(COMMONOBJ)

serverobj : $(SERVEROBJ)

testobj : $(TESTOBJ)

cleanobj : 
	$(RM) $(foreach dir,$(COMMONDIR),$(dir)*.o)
	$(RM) $(foreach dir,$(SERVERDIR),$(dir)*.o)
	$(RM) $(foreach dir,$(TESTDIR),$(dir)*.o)

cleandep : 
	$(RM) *.d
	$(RM) $(foreach dir,$(COMMONDIR),$(dir)*.d)
	$(RM) $(foreach dir,$(SERVERDIR),$(dir)*.d)
	$(RM) $(foreach dir,$(TESTDIR),$(dir)*.d)

cleandist : cleanobj cleandep

clean : cleandist
	$(RM) $(BINDIR)$(SERVER_TARGET)
	$(RM) $(TEST_TARGET_LIST)

love: clean all

sinclude $(SERVERDEP) $(COMMONDEP) $(TESTDEP)

.PHONY : all link test force commonobj serverobj testobj cleanobj cleandep cleandist clean love  

