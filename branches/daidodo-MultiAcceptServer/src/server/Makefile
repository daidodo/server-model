TARGET = $(OBJ:_server.o=_server)
SVROBJ = $(filter-out %_server.o, $(OBJ))

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp
sinclude $(BASEDIR)/inc.mak

all: comm frame $(TARGET)

release: commrelease framerelease $(TARGET)

comm:
	$(MAKE) -C $(COMMON) all

commrelease:
	$(MAKE) -C $(COMMON) release

frame:
	$(MAKE) -C $(FRAME) all

framerelease:
	$(MAKE) -C $(FRAME) release

%_server: %_server.o $(SVROBJ) $(LIBFRAME) $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

cleanall: clean
	$(RM) tags

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},cleanall)
sinclude $(DEP)
endif
endif

.PHONY: all release comm commrelease frame framerelease clean cleanall

.SECONDARY: $(OBJ)
