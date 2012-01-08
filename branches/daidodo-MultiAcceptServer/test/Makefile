TARGET = $(OBJ:_test.o=_test)

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp
MYSQL := -lmysqlclient_r -lz
sinclude $(BASEDIR)/inc.mak

all: comm $(TARGET)

release: commrelease $(TARGET)

comm:
	$(MAKE) -C $(COMMON) all

commrelease:
	$(MAKE) -C $(COMMON) release

%_test: %_test.o $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

cleanall: clean
	$(RM) tags

run:
	@./run_test.sh $(TARGET)

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},cleanall)
ifneq (${MAKECMDGOALS},run)
sinclude $(DEP)
endif
endif
endif

.PHONY: all release comm commrelease clean cleanall run

.SECONDARY: $(OBJ)
