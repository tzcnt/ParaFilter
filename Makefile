BINARY_DEBUG=./target/debug/bin-debug
BINARY_MPI=./target/debug/bin-debug-mpi
BINARY_RELEASE=./target/release/bin-release
BINARY_TEST=./target/test/bin-test
BINARY_BENCH=./target/bench/bin-bench
CODEDIRS=. ./src
TESTDIRS=./test
BENCHDIRS=./benchmark
INCDIRS=. ./src/include

CC=g++
OPT=-O0

DEBUGFLAGS=-g -ggdb
RELEASE_FLAGS=-O3 -march=native -mtune=native -flto -fuse-linker-plugin -ftree-vectorize
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -fopenmp $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS_TEST=-lgtest -lgtest_main -pthread  # Linking Google Test libs
LDFLAGS_BENCH=-lbenchmark -lpthread # Linking Google Benchmark

CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
TESTFILES=$(foreach D,$(TESTDIRS),$(wildcard $(D)/*.cpp))
BENCHFILES=$(foreach D,$(BENCHDIRS),$(wildcard $(D)/*.cpp))

OBJECTS=$(patsubst %.cpp,%.o,$(CPPFILES))
OBJECTS_MPI=$(patsubst %.cpp,%_mpi.o,$(CPPFILES))
OBJECTS_NOMAIN=$(patsubst %.cpp,%.o,$(filter-out ./src/main.cpp, $(CPPFILES)))  # Excludes main.cpp
TESTOBJECTS=$(patsubst %.cpp,%.o,$(TESTFILES))
BENCHOBJECTS=$(patsubst %.cpp,%.o,$(BENCHFILES))

DEPFILES=$(patsubst %.cpp,%.d,$(CPPFILES) $(TESTFILES) $(BENCHFILES))

NUM_PROCESS=4

all: $(BINARY_DEBUG)

debug: $(BINARY_DEBUG)
	$(BINARY_DEBUG)

$(BINARY_DEBUG): $(OBJECTS)
	$(CC) $(DEBUGFLAGS) -fopenmp -o $@ $^

$(BINARY_MPI): $(OBJECTS_MPI)
	mpic++ $(DEBUGFLAGS) -DUSE_MPI=1 -fopenmp -o $@ $^

release: $(BINARY_RELEASE)
	$(BINARY_RELEASE)

$(BINARY_RELEASE): $(OBJECTS)
	$(CC) $(RELEASE_FLAGS) -fopenmp -o $@ $^

$(BINARY_TEST): $(OBJECTS_NOMAIN) $(TESTOBJECTS)
	$(CC) $(LDFLAGS_TEST) -fopenmp -o $@ $^

$(BINARY_BENCH): $(OBJECTS_NOMAIN) $(BENCHOBJECTS)
	$(CC) $(RELEASE_FLAGS) $(LDFLAGS_BENCH) -fopenmp -o $@ $^

mpi_run: $(BINARY_MPI)
	mpirun -n $(NUM_PROCESS) $(BINARY_MPI)

%_mpi.o:%.cpp
	mpic++ $(CFLAGS) -DUSE_MPI=1 -c -o $@ $<

%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_DEBUG) $(BINARY_RELEASE) $(BINARY_TEST) $(BINARY_BENCH) $(BINARY_MPI) $(OBJECTS) $(OBJECTS_MPI) $(TESTOBJECTS) $(BENCHOBJECTS) $(DEPFILES)

bench: $(BINARY_BENCH)
	./$(BINARY_BENCH)

test: $(BINARY_TEST)
	./$(BINARY_TEST)

# include the dependencies
-include $(DEPFILES)

.PHONY: all clean test bench
