BINARY_DEBUG=./target/debug/bin-debug
BINARY_RELEASE=./target/release/bin-release
BINARY_TEST=./target/test/bin-test
CODEDIRS=. ./src
TESTDIRS=./test
INCDIRS=. ./src/include

CC=g++
OPT=-O0

DEBUGFLAGS=-g -ggdb
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra $(DEBUGFLAGS) $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS=-lgtest -lgtest_main -pthread  # Linking Google Test libs

CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
TESTFILES=$(foreach D,$(TESTDIRS),$(wildcard $(D)/*.cpp))
OBJECTS=$(patsubst %.cpp,%.o,$(CPPFILES))
OBJECTS_NOMAIN=$(patsubst %.cpp,%.o,$(filter-out ./src/main.cpp, $(CPPFILES)))  # Excludes main.cpp
TESTOBJECTS=$(patsubst %.cpp,%.o,$(TESTFILES))
DEPFILES=$(patsubst %.cpp,%.d,$(CPPFILES) $(TESTFILES))

all: $(BINARY_DEBUG) test

$(BINARY_DEBUG): $(OBJECTS)
	$(CC) -o $@ $^

$(BINARY_RELEASE): $(OBJECTS)
	$(CC) -o $@ $^

$(BINARY_TEST): $(OBJECTS_NOMAIN) $(TESTOBJECTS)
	$(CC) -o $@ $^ $(LDFLAGS)

%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_DEBUG) $(BINARY_RELEASE) $(BINARY_TEST) $(OBJECTS) $(TESTOBJECTS) $(DEPFILES)

test: $(BINARY_TEST)
	./$(BINARY_TEST)

# include the dependencies
-include $(DEPFILES)

.PHONY: all clean test
