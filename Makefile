BINARY_DEBUG=./target/debug/bin-debug
BINARY_RELEASE=./target/release/bin-release
BINARY_TEST=./target/test/bin-test
BINARY_BENCH=./target/bench/bin-bench
CODEDIRS=. ./src
TESTDIRS=./test
BENCHDIRS=./benchmark/
INCDIRS=. ./src/include

CC=g++
OPT=-O0

DEBUGFLAGS=-g -ggdb
RELEASE_FLAGS=-O3 -march=native -mtune=native -flto -fuse-linker-plugin
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -fopenmp $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS_TEST=-lgtest -lgtest_main -pthread  # Linking Google Test libs
LDFLAGS_BENCH=-lbenchmark -lpthread # Linking Google Benchmark

CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
TESTFILES=$(foreach D,$(TESTDIRS),$(wildcard $(D)/*.cpp))
BENCHFILES=$(foreach D,$(BENCHDIRS),$(wildcard $(D)/*.cpp))

OBJECTS=$(patsubst %.cpp,%.o,$(CPPFILES))
OBJECTS_NOMAIN=$(patsubst %.cpp,%.o,$(filter-out ./src/main.cpp, $(CPPFILES)))  # Excludes main.cpp
TESTOBJECTS=$(patsubst %.cpp,%.o,$(TESTFILES))
BENCHOBJECTS=$(patsubst %.cpp,%.o,$(BENCHFILES))

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
	$(CC) $(LDFLAGS_TEST) -fopenmp -o $@ $^


$(BINARY_BENCH): $(OBJECTS_NOMAIN) $(BENCHOBJECTS)
	$(CC)  $(LDFLAGS_BENCH) -fopenmp -o $@ $^

%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_DEBUG) $(BINARY_RELEASE) $(BINARY_TEST) $(BINARY_BENCH) $(OBJECTS) $(TESTOBJECTS) $(BENCHOBJECTS) $(DEPFILES)

bench: $(BINARY_BENCH)
	./$(BINARY_BENCH)

test: $(BINARY_TEST)
	./$(BINARY_TEST)

# include the dependencies
-include $(DEPFILES)

.PHONY: all clean test bench
