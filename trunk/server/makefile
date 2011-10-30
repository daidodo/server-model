TARGET = $(BASEDIR)/bin/server.out

# common defines
BASEDIR := ..
SRC_DIR := . ./frame
SRC_SUFFIX := cpp
sinclude $(BASEDIR)/inc.mak

COMM_DIR := $(BASEDIR)/common
LIBCOMM := $(COMM_DIR)/libcommon.a

all: $(TARGET)

$(TARGET): $(OBJ) $(LIBCOMM)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LIB)

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

release: all

ifneq (${MAKECMDGOALS},clean)
sinclude $(DEP)
endif

.PHONY: all clean
