BINARY_DEBUG=./target/debug/bin-debug
BINARY_RELEASE=./target/release/bin-release
CODEDIRS=. ./src
INCDIRS=. ./src/include

CC=g++
OPT=-O0

DEBUGFLAGS=-g -ggdb
# generate files that encode make rules for the .h dependencies
DEPFLAGS=-MP -MD
# automatically add the -I onto each include directory
CFLAGS=-Wall -Wextra $(DEBUGFLAGS) $(foreach D,$(INCDIRS),-I$(D)) $(OPT) $(DEPFLAGS)

# for-style iteration (foreach) and regular expression completions (wildcard)
CPPFILES=$(foreach D,$(CODEDIRS),$(wildcard $(D)/*.cpp))
# regular expression replacement
OBJECTS=$(patsubst %.cpp,%.o,$(CPPFILES))
DEPFILES=$(patsubst %.cpp,%.d,$(CPPFILES))

all: $(BINARY_DEBUG)

$(BINARY_DEBUG): $(OBJECTS)
	$(CC) -o $@ $^

$(BINARY_RELEASE): $(OBJECTS)
	$(CC) -o $@ $^

# only want the .c file dependency here, thus $< instead of $^.
%.o:%.cpp
	$(CC) $(CFLAGS) -c -o $@ $<

clean:
	rm -rf $(BINARY_DEBUG) $(BINARY_RELEASE) $(OBJECTS) $(DEPFILES)

# include the dependencies
-include $(DEPFILES)

# add .PHONY so that the non-targetfile - rules work even if a file with the same name exists.
.PHONY: all clean

