# Makefile

# --- Source Files and Target ---
SRCS_CPP = lexer.cpp parser.cpp main.cpp
SRCS_H = lexer.h parser.h
SRCS = $(SRCS_CPP) $(SRCS_H) # All source files, including headers
TARGET = myparse
OBJS = $(SRCS_CPP:.cpp=.o) # Object files are generated only from .cpp files

# --- Compiler and Flags ---
CXX ?= g++ # Use g++ as the default C++ compiler
# CXXFLAGS: Default flags for C++ compilation
# Using := for immediate expansion and safer re-assignments
CXXFLAGS := -std=c++23 -Wall $(INCDIR)

# LDFLAGS: Default linker flags
LDFLAGS :=

# INCDIR: Include directories
INCDIR := -I. # Current directory as include path

# LIBDIR: Library directories (currently empty)
LIBDIR :=

# --- Extra Flags for Command Line Overrides (Recommended Practice) ---
# Define variables to allow easy addition of flags from the command line
# These are empty by default, but can be set by `make EXTRA_CXXFLAGS="-DTEST"`
EXTRA_CXXFLAGS ?=
EXTRA_LDFLAGS ?=

# --- Final Flags used in Compilation/Linking ---
# Combine default flags with any extra flags provided
FINAL_CXXFLAGS := $(CXXFLAGS) $(EXTRA_CXXFLAGS)
FINAL_LDFLAGS := $(LDFLAGS) $(EXTRA_LDFLAGS)

# --- General Rules ---
.PHONY: all clean lexer lexer_d test testg

# Rule to compile .cpp files into .o files
# Use $< for the prerequisite (e.g., lexer.cpp) and $@ for the target (e.g., lexer.o)
%.o: %.cpp $(SRCS_H) # Depend on headers for recompilation if headers change
	$(CXX) $(FINAL_CXXFLAGS) -c $< -o $@

# Rule for the main executable (myparse)
$(TARGET): $(OBJS)
	$(CXX) $(FINAL_CXXFLAGS) $(FINAL_LDFLAGS) $(OBJS) -o $(TARGET)

# --- Specific Build Targets ---

# Default target: builds the main executable
all: $(TARGET)

# Clean up generated files
clean:
	@rm -f $(OBJS) $(TARGET) lexer 2>/dev/null
#	-@rm -f *.o lexer 2>/dev/null # Alternative with silent failure

# Target to build only the lexer.cpp into an executable (for testing lexer in isolation)
# This uses the FINAL_CXXFLAGS, so you can control it with EXTRA_CXXFLAGS
lexer: lexer.cpp $(SRCS_H) # lexer.cpp also depends on lexer.h
	$(CXX) $(FINAL_CXXFLAGS) $(FINAL_LDFLAGS) -DTEST lexer.cpp -o lexer

# Target for lexer debug build
lexer_d: lexer.cpp $(SRCS_H)
	$(CXX) -std=c++23 -g -O0 $(INCDIR) $(LDFLAGS) -DTEST lexer.cpp -o lexer

# Target for main executable with TEST define (using FINAL_CXXFLAGS)
test: $(TARGET)
	# This target is usually used to run tests, not just build.
	# The $(TARGET) dependency ensures it's built before this 'test' rule runs
	# If you want to rebuild with -DTEST, you should use EXTRA_CXXFLAGS for $(TARGET)
	# For example: make EXTRA_CXXFLAGS="-DTEST" myparse
	# Or, if you want a dedicated 'test' build:
	$(CXX) $(FINAL_CXXFLAGS) $(FINAL_LDFLAGS) -DTEST $(SRCS_CPP) -o $(TARGET)_test

# Target for main executable with TEST define and debug info
testg: $(TARGET)
	# Similar to 'test', if you want to rebuild with -DTEST and -g, specify via EXTRA_CXXFLAGS
	# For example: make EXTRA_CXXFLAGS="-DTEST -g -O0" myparse
	# Or, for a dedicated 'testg' build:
	$(CXX) $(FINAL_CXXFLAGS) $(FINAL_LDFLAGS) -g -O0 -DTEST $(SRCS_CPP) -o $(TARGET)_testg
