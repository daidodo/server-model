COMMONDIR := common
FRAMEDIR := frame
SERVERDIR := server
TESTDIR := test
PFTESTDIR := pf_test

all: debug

debug:
	$(MAKE) -C $(SERVERDIR) debug

release:
	$(MAKE) -C $(SERVERDIR) release

clean:
	$(MAKE) -C $(COMMONDIR) clean
	$(MAKE) -C $(FRAMEDIR) clean
	$(MAKE) -C $(SERVERDIR) clean

test:
	$(MAKE) -C $(TESTDIR) all

testrelease:
	$(MAKE) -C $(TESTDIR) release

cleanall:
	$(MAKE) -C $(COMMONDIR) cleanall
	$(MAKE) -C $(FRAMEDIR) cleanall
	$(MAKE) -C $(SERVERDIR) cleanall
	$(MAKE) -C $(TESTDIR) cleanall
	$(MAKE) -C $(PFTESTDIR) cleanall
	$(RM) tags

love: clean all

lines:
	@find . -regex ".*\.[ch]p?p?" | grep -vw "lzo" | grep -vw "quicklz" | xargs wc -l #| sort -h

.PHONY : all clean test cleanall love lines release

