BINARY_SEQ=./target/debug/bin-debug-seq
BINARY_OPENMP=./target/debug/bin-debug-openmp
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

OPENMP_FLAGS=-fopenmp -DOPENMP=1
DEUBG_FLAGS=-g -ggdb
RELEASE_FLAGS=-O3 -march=native -mtune=native -flto -fuse-linker-plugin -ftree-vectorize
DEPFLAGS=-MP -MD
CFLAGS=-Wall -Wextra -fopenmp $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)
LDFLAGS_TEST=-lgtest -lgtest_main -pthread  # Linking Google Test libs
LDFLAGS_BENCH=-lbenchmark -lpthread # Linking Google Benchmark

CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
TESTFILES=$(foreach D,$(TESTDIRS),$(wildcard $(D)/*.cpp))
BENCHFILES=$(foreach D,$(BENCHDIRS),$(wildcard $(D)/*.cpp))

OBJECTS=$(patsubst %.cpp,%_seq.o,$(CPPFILES))
OBJECTS_OPENMP=$(patsubst %.cpp,%_openmp.o,$(CPPFILES))
OBJECTS_MPI=$(patsubst %.cpp,%_mpi.o,$(CPPFILES))
OBJECTS_NOMAIN=$(patsubst %.cpp,%.o,$(filter-out ./src/main.cpp, $(CPPFILES)))  # Excludes main.cpp
TESTOBJECTS=$(patsubst %.cpp,%.o,$(TESTFILES))
BENCHOBJECTS=$(patsubst %.cpp,%.o,$(BENCHFILES))

DEPFILES=$(patsubst %.cpp,%.d,$(CPPFILES) $(TESTFILES) $(BENCHFILES))

NUM_PROCESS ?= 4

all: $(BINARY_SEQ)

debug: $(BINARY_SEQ)
	$(BINARY_SEQ)

seq: $(BINARY_SEQ)
	$(BINARY_SEQ)

openmp: $(BINARY_OPENMP)
	$(BINARY_OPENMP)

release: $(BINARY_RELEASE)
	$(BINARY_RELEASE)

mpi_run: $(BINARY_MPI)
	mpirun  --map-by :OVERSUBSCRIBE -n $(NUM_PROCESS) $(BINARY_MPI)

$(BINARY_SEQ): $(OBJECTS)
	$(CC) $(DEUBG_FLAGS) -o $@ $^

$(BINARY_OPENMP): $(OBJECTS_OPENMP)
	$(CC) $(DEUBG_FLAGS) $(OPENMP_FLAGS) -o $@ $^

$(BINARY_MPI): $(OBJECTS_MPI)
	mpic++ $(RELEASE_FLAGS) $(OPENMP_FLAGS) -DUSE_MPI=1 -o $@ $^

$(BINARY_TEST): $(OBJECTS_NOMAIN) $(TESTOBJECTS)
	$(CC) $(LDFLAGS_TEST) $(OPENMP_FLAGS) -o $@ $^

$(BINARY_BENCH): $(OBJECTS_NOMAIN) $(BENCHOBJECTS)
	$(CC) $(RELEASE_FLAGS) $(LDFLAGS_BENCH) $(OPENMP_FLAGS) -o $@ $^

$(BINARY_RELEASE): $(OBJECTS_OPENMP)
	$(CC) $(RELEASE_FLAGS) $(OPENMP_FLAGS) -o $@ $^

%_mpi.o:%.cpp
	mpic++ $(OPENMP_FLAGS) $(CFLAGS) -DUSE_MPI=1 -c -o $@ $<

%_openmp.o:%.cpp
	$(CC) -DOPENMP $(CFLAGS) -c -o $@ $<

%_seq.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_SEQ) $(BINARY_OPENMP) $(BINARY_RELEASE) $(BINARY_TEST) $(BINARY_BENCH) $(BINARY_MPI) $(OBJECTS) $(OBJECTS_OPENMP) $(OBJECTS_MPI) $(TESTOBJECTS) $(BENCHOBJECTS) $(DEPFILES)

bench: $(BINARY_BENCH)
	$(BINARY_BENCH)

bench-mpi: $(BINARY_MPI)
	@sh -c 'time mpirun --map-by :OVERSUBSCRIBE -n 2 $(BINARY_MPI)'
	@sh -c 'time mpirun --map-by :OVERSUBSCRIBE -n 4 $(BINARY_MPI)'
	@sh -c 'time mpirun --map-by :OVERSUBSCRIBE -n 8 $(BINARY_MPI)'
	@sh -c 'time mpirun --map-by :OVERSUBSCRIBE -n 16 $(BINARY_MPI)'

bench-report: $(BINARY_MPI) $(BINARY_BENCH)
	@echo "Generating Report..."
	@sh -c 'make bench >> bench_report.txt'
	@sh -c 'make bench-mpi 2>> bench_report.txt'
	@sh -c 'sed "s/make.*//g"'


test: $(BINARY_TEST)
	$(BINARY_TEST)

# include the dependencies
-include $(DEPFILES)

.PHONY: all clean test bench
