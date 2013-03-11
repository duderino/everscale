DIRS=\
stack\
stack/test\
server\
client

all: 
	@for d in $(DIRS); do \
		echo; \
		cd $(CURDIR)/$$d && $(MAKE) || exit 1; \
		echo; \
	done

clean:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) clean || exit 1; \
	done

test:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) test || exit 1; \
	done


remake: clean all
