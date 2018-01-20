.PHONY: default test

TARGET = TestRangeLock
LIBS = -L/usr/local/lib -lgtest

default: $(TARGET)

$(TARGET): TestRangeLock.cxx RangeLock.h
	$(CXX) -g -o $@ $< $(LIBS)

test: TestRangeLock
	./$(TARGET)

clean:
	rm -f $(TARGET)