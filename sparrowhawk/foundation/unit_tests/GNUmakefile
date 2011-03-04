TESTS = \
ESFBuddyAllocatorTestMain.mak \
ESFDiscardAllocatorTestMain.mak \
ESFFixedAllocatorTestMain.mak \
ESFListTestMain.mak \
ESFLockableTestMain.mak \
ESFMapTestMain.mak \
ESFSmartPointerTestMain.mak \
ESFSharedCounterTestMain.mak \
ESFSharedQueueTestMain.mak \
ESFBufferTestMain.mak

all:
	@for d in $(TESTS); do \
		echo; \
		$(MAKE) -f $$d || exit 1; \
	done

clean:
	@for d in $(TESTS); do \
		echo; \
		$(MAKE) -f $$d clean || exit 1; \
	done
	
run:
	@for d in $(TESTS); do \
		echo; \
		$(MAKE) -f $$d run || exit 1; \
	done

test: run
	
remake: clean all
