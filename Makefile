.PHONY: default test

TARGET = TestRangeLock

LIBS = -L/usr/local/lib -lgtest
CXXFLAGS += -g

default: $(TARGET)

$(TARGET): TestRangeLock.cxx RangeLock.h
	$(CXX) -o $@ $(CXXFLAGS) $< $(LIBS)

test: TestRangeLock
	./$(TARGET)

clean:
	rm -f $(TARGET)
