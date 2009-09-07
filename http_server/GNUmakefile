DIRS=\
stack\
stack/test\
server\
client

all: 
	@for d in $(DIRS); do \
		echo; \
		cd $(CURDIR)/$$d && $(MAKE); \
		echo; \
	done

clean:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) clean; \
	done

remake: clean all
