BINARY_DEBUG=./target/debug/bin-debug
BINARY_RELEASE=./target/release/bin-release
BINARY_TEST=./target/test/bin-test
CODEDIRS=. ./src
TESTDIRS=./test
INCDIRS=. ./src/include

CC=g++
OPT=-O0

DEBUGFLAGS=-g -ggdb
RELEASE_FLAGS=-O2 -march=native
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -fopenmp $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS_TEST=-lgtest -lgtest_main -pthread  # Linking Google Test libs

CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
TESTFILES=$(foreach D,$(TESTDIRS),$(wildcard $(D)/*.cpp))
OBJECTS=$(patsubst %.cpp,%.o,$(CPPFILES))
OBJECTS_NOMAIN=$(patsubst %.cpp,%.o,$(filter-out ./src/main.cpp, $(CPPFILES)))  # Excludes main.cpp
TESTOBJECTS=$(patsubst %.cpp,%.o,$(TESTFILES))
DEPFILES=$(patsubst %.cpp,%.d,$(CPPFILES) $(TESTFILES))

all: $(BINARY_DEBUG)


debug: $(BINARY_DEBUG)
	$(BINARY_DEBUG)

$(BINARY_DEBUG): $(OBJECTS)
	$(CC) $(DEBUGFLAGS) -fopenmp -o $@ $^

release: $(BINARY_RELEASE)
	$(BINARY_RELEASE)

$(BINARY_RELEASE): $(OBJECTS)
	$(CC) $(RELEASE_FLAGS) -fopenmp -o $@ $^

$(BINARY_TEST): $(OBJECTS_NOMAIN) $(TESTOBJECTS)
	$(CC) -fopenmp -o $@ $^ $(LDFLAGS_TEST)

%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_DEBUG) $(BINARY_RELEASE) $(BINARY_TEST) $(OBJECTS) $(TESTOBJECTS) $(DEPFILES)

test: $(BINARY_TEST)
	./$(BINARY_TEST)

# include the dependencies
-include $(DEPFILES)

.PHONY: all clean test
