TARGET := libframe.a

# common defines
BASEDIR := ..
SRC_DIR := .
SRC_SUFFIX := cpp
sinclude $(BASEDIR)/inc.mak

all: debug

debug: $(TARGET)

release: $(TARGET)

$(TARGET): $(OBJ)
	$(AR) $(ARFLAGS) $@ $^

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

.PHONY: all debug release clean cleanall
