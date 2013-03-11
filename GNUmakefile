DIRS=\
dependencies/sparrowhawk\
raw_echo_server\
http_server\
http_server/stack/test

all: init
	@for d in $(DIRS); do \
		echo; \
		cd $(CURDIR)/$$d && $(MAKE) || exit 1; \
		echo; \
	done

init:
	@git submodule init

clean:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) clean || exit 1; \
	done

test:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) test || exit 1; \
	done

remake: clean all

nodeps:
	@find . -name '.depends' -exec rm -rf {} \;
