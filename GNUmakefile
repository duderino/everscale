DIRS=\
sparrowhawk\
raw_echo_server\
http_server\
http_server/stack/test

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

remake: clean all

nodeps:
	@find . -name '.depends' -exec rm -rf {} \;
