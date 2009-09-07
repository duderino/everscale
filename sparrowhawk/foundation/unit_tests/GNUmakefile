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
		$(MAKE) -f $$d; \
	done

clean:
	@for d in $(TESTS); do \
		echo; \
		$(MAKE) -f $$d clean; \
	done
	
run:
	@for d in $(TESTS); do \
		echo; \
		$(MAKE) -f $$d run; \
	done
	
remake: clean all
