.PHONY: default test

LIBS = -L/usr/local/lib -lgtest

default: TestRangeLock

TestRangeLock: TestRangeLock.cxx RangeLock.h
	$(CXX) -o $@ $< $(LIBS)

test: TestRangeLock
	./TestChannel

clean:
	rm -f TestRangeLock
