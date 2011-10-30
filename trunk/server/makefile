TARGET = $(BASEDIR)/bin/server.out

# common defines
BASEDIR := ..
SRC_DIR := . ./frame
SRC_SUFFIX := cpp
sinclude $(BASEDIR)/inc.mak

COMM_DIR := $(BASEDIR)/common
LIBCOMM := $(COMM_DIR)/libcommon.a

all: comm $(TARGET)

release: commrelease $(TARGET)

comm:
	$(MAKE) -C $(COMM_DIR) all

commrelease:
	$(MAKE) -C $(COMM_DIR) release

$(TARGET): $(OBJ) $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)


ifneq (${MAKECMDGOALS},clean)
sinclude $(DEP)
endif

.PHONY: all release comm commrelease clean
