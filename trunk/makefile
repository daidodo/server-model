COMMONDIR := common
SERVERDIR := server
TESTDIR := test

all:
	$(MAKE) -C $(COMMONDIR) all
	$(MAKE) -C $(SERVERDIR) all

release:
	$(MAKE) -C $(COMMONDIR) release
	$(MAKE) -C $(SERVERDIR) release

clean:
	$(MAKE) -C $(COMMONDIR) clean
	$(MAKE) -C $(SERVERDIR) clean

test:
	$(MAKE) -C $(TESTDIR) all

testrelease:
	$(MAKE) -C $(TESTDIR) release

cleantest:
	$(MAKE) -C $(TESTDIR) clean

cleanall: clean cleantest
	$(RM) tags

love: clean all

lines:
	@find . -regex ".*\.[ch]p?p?" | xargs wc -l

.PHONY : all clean test cleantest cleanall love lines release

