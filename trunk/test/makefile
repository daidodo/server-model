TARGET = $(OBJ:_test.o=_test)

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp
ZIP := -lz
sinclude $(BASEDIR)/inc.mak

COMM_DIR := $(BASEDIR)/common
LIBCOMM := $(COMM_DIR)/libcommon.a

all: comm $(TARGET)

release: commrelease $(TARGET)

comm:
	$(MAKE) -C $(COMM_DIR) all

commrelease:
	$(MAKE) -C $(COMM_DIR) release

%_test: %_test.o $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

run:
	@./run_test.sh $(TARGET)

ifneq (${MAKECMDGOALS},clean)
ifneq (${MAKECMDGOALS},run)
sinclude $(DEP)
endif
endif

.PHONY: all release comm commrelease clean

.SECONDARY: $(OBJ)
