COMMONDIR      :=common/ common/lzo/
SERVERDIR      :=server/ server/frame/

BINDIR         :=bin/
SERVER_TARGET  :=Server.out
TESTDIR        :=test/
TEST_TARGET    :=test.out
SRC_SUFFIX     :=cpp c

    #external
EXTERN_FLAGS   :=#-pg
EXTERN__LIB    :=

####�����ǿ�ѡ�ı������
    #------use logger(-DLOGGER) or not
LOG            :=-DLOGGER
    #------use logsys(LOGSYS=-DLOGSYS) or log4cpp(LOG4CLIB=-llog4cplus)
LOGSYS         :=-DLOGSYS
#LOG4CLIB       :=-llog4cplus
    #------debug mode or not(-DNDEBUG)
#RELEASE        :=-DNDEBUG -O2
DEBUG           :=
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
INCLUDE        :=-I./ -I/usr/local/ssl/include/ -I/usr/local/mysql/include/
CXXFLAGS       :=-Wall -g $(INCLUDE) -D_REENTRANT $(LOG) $(LOGSYS) $(DEBUG) $(RELEASE) $(EPOLL) $(EXTERN_FLAGS)
LIB            :=-L/usr/local/ssl/lib/ -L/usr/local/mysql/lib -L/usr/lib -L/usr/local/lib -L./lib -lstdc++ -lpthread -lrt $(MYSQL) $(LOG4CLIB) $(ZIP) $(CRYPTO) $(EXTERN__LIB)
CFLAGS         :=$(CXXFLAGS)

####������һЩ���õĺ���

#������$(1)����$@��ԭ���ǣ��ڹ��������$@��������
#��ÿ��TEST_TARGET�ļ����õ������.o�ļ��б�
TEST_OBJS=$(filter $(dir $(1))%.o,$(TESTOBJ))

#����ÿ��TEST_TARGET�Ĺ���
define BUILD_TEST
$(1) : $$(COMMONOBJ) $$(call TEST_OBJS,$(1))
	$$(CXX) $$^ $$(CXXFLAGS) -o $$@ $$(LIB)
endef

#�õ�����SRC_SUFFIX��׺��Դ�����ļ�
GETSRC=$(foreach dir,$(1),$(wildcard $(addprefix $(dir)*.,$(SRC_SUFFIX))))

#�õ�.o�ļ�
GETOBJ=$(foreach suffix,$(SRC_SUFFIX),$(filter %.o,$($(1):.$(suffix)=.o)))

####�����ǹ��̴�����������äĿ�޸�

TESTSRC:=$(call GETSRC,$(TESTDIR))
SERVERSRC:=$(call GETSRC,$(SERVERDIR))
COMMONSRC:=$(call GETSRC,$(COMMONDIR))

TESTOBJ:=$(call GETOBJ,TESTSRC)
SERVEROBJ:=$(call GETOBJ,SERVERSRC)
COMMONOBJ:=$(call GETOBJ,COMMONSRC)

TESTDEP:=$(TESTOBJ:.o=.d)
SERVERDEP:=$(SERVEROBJ:.o=.d)
COMMONDEP:=$(COMMONOBJ:.o=.d)

HEADERSRC := $(foreach dir,$(COMMONDIR),$(wildcard $(dir)*.h)) $(foreach dir,$(SERVERDIR),$(wildcard $(dir)*.h))

CXXFLAGS+=-MD
CFLAGS+=-MD

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

cleanbin :
	$(RM) $(BINDIR)$(SERVER_TARGET)
	$(RM) $(TEST_TARGET_LIST)

cleandist : cleanobj cleandep

clean : cleandist cleanbin

love: cleanobj cleanbin all

lines:
	@echo $(SERVERSRC) $(COMMONSRC) $(HEADERSRC) | xargs wc -l

.PHONY : all link test force commonobj serverobj testobj cleanobj cleandep cleanbin cleandist clean love lines

ifneq (${MAKECMDGOALS},force)
ifneq (${MAKECMDGOALS},cleanobj)
ifneq (${MAKECMDGOALS},cleandep)
ifneq (${MAKECMDGOALS},cleandist)
ifneq (${MAKECMDGOALS},cleanbin)
ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},lines)
sinclude $(SERVERDEP) $(COMMONDEP) $(TESTDEP)
endif
endif
endif
endif
endif
endif
endif

