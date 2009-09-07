DIRS=\
test_framework\
foundation\
foundation/unit_tests

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

fully_clean:
	@for d in $(DIRS); do \
		cd $(CURDIR)/$$d && $(MAKE) fully_clean; \
	done 

remake: clean all
