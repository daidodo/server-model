COMMONDIR := common
SERVERDIR := server
TESTDIR := test

all:
	$(MAKE) -C $(SERVERDIR) all

release:
	$(MAKE) -C $(SERVERDIR) release

clean:
	$(MAKE) -C $(COMMONDIR) clean
	$(MAKE) -C $(SERVERDIR) clean

test:
	$(MAKE) -C $(TESTDIR) all

testrelease:
	$(MAKE) -C $(TESTDIR) release

cleanall:
	$(MAKE) -C $(COMMONDIR) cleanall
	$(MAKE) -C $(SERVERDIR) cleanall
	$(MAKE) -C $(TESTDIR) cleanall
	$(RM) tags

love: clean all

lines:
	@find . -regex ".*\.[ch]p?p?" | xargs wc -l

.PHONY : all clean test cleanall love lines release

