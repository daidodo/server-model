TARGET = $(OBJ:_test.o=_test)

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp c
MYSQL := -lmysqlclient_r -lz
sinclude $(BASEDIR)/inc.mak

all: comm $(TARGET)

release: commrelease $(TARGET)

comm:
	$(MAKE) -C $(COMMON) all

commrelease:
	$(MAKE) -C $(COMMON) release

%_test: %_test.o sv_pack.o $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(MAKE) -C $(COMMON) clean
	$(RM) $(TARGET)

cleanall: clean
	$(RM) tags

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},cleanall)
ifneq (${MAKECMDGOALS},run)
sinclude $(DEP)
endif
endif
endif

.PHONY: all release comm commrelease clean cleanall

.SECONDARY: $(OBJ)
