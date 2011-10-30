TARGET = $(OBJ:_test.o=_test)

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp
sinclude $(BASEDIR)/inc.mak

COMM_DIR := $(BASEDIR)/common
LIBCOMM := $(COMM_DIR)/libcommon.a

all: $(TARGET)

%_test: %_test.o $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

run:
	@./run_test.sh $(TARGET)

release: all

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},run)
sinclude $(DEP)
endif
endif

.PHONY: all clean
