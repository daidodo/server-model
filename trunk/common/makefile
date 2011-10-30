TARGET := libcommon.a

# common defines
BASEDIR := ..
SRC_DIR := . ./lzo
SRC_SUFFIX := cpp c
sinclude $(BASEDIR)/inc.mak

all: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

clean:
	$(RM) $(foreach dir, $(SRC_DIR), $(dir)/*.o $(dir)/*.d)
	$(RM) $(TARGET)

release: all

ifneq (${MAKECMDGOALS},clean)
sinclude $(DEP)
endif

.PHONY: all clean
