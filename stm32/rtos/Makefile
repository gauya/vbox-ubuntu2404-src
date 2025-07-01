# Makefile - CORRECTED VERSION (AGAIN!)
# arm-none-eabi-g++ -mcpu=cortex-m4 -mthumb -o main.o -c main.cpp

# --- Source Files and Target ---
SRCS_CPP = rtos.c mutex.c semaphore.c
TARGET = rtos
OBJS = $(SRCS_CPP:.cpp=.o)

# --- Compiler and Flags ---
CXX ?= arm-none-eabi-gcc # Use g++ as the default C++ compiler

# CXXFLAGS_COMPILE: Flags specifically for compilation (.cpp to .o)
# -std=c++23: Use C++23 standard
# -Wall: Enable all standard warnings
# -MMD: Generate dependency files (.d files) for each .o file.
#       This is crucial for correct header dependency tracking.
# $(INCDIR): Include directories (e.g., -I.)
CXXFLAGS_COMPILE := -std=c++23 -Wall -mcpu=cortex-m4 -mthumb $(INCDIR)

# CXXFLAGS_LINK: Flags specifically for linking (.o to executable)
# Usually just includes library paths or other linker-specific flags.
CXXFLAGS_LINK := $(INCDIR) # Keep include paths for consistency if needed, but not strictly for linking itself

# LDFLAGS: Default linker flags (e.g., for specific libraries like -lpqxx)
LDFLAGS :=

# INCDIR: Include directories
INCDIR := -I. # Current directory as include path

# LIBDIR: Library directories (currently empty, add -L/path/to/lib if needed)
LIBDIR :=

# --- Extra Flags for Command Line Overrides ---
EXTRA_CXXFLAGS_COMPILE ?=
EXTRA_CXXFLAGS_LINK ?=
EXTRA_LDFLAGS ?=

# --- Final Flags used in Compilation/Linking ---
FINAL_CXXFLAGS_COMPILE := $(CXXFLAGS_COMPILE) $(EXTRA_CXXFLAGS_COMPILE)
FINAL_CXXFLAGS_LINK := $(CXXFLAGS_LINK) $(EXTRA_CXXFLAGS_LINK)
FINAL_LDFLAGS := $(LDFLAGS) $(EXTRA_LDFLAGS)

# --- General Rules ---
.PHONY: all clean

# Include automatically generated dependency files.
# These files will list header dependencies for each .o file.
-include $(OBJS:.o=.d)

# Rule to compile .cpp files into .o files
# Use the compilation-specific flags here
%.o: %.cpp
	$(CXX) $(FINAL_CXXFLAGS_COMPILE) -c $< -o $@

# Rule for the main executable
# Use the linking-specific flags here
$(TARGET): $(OBJS)
	$(CXX) $(FINAL_CXXFLAGS_LINK) $(FINAL_LDFLAGS) $(OBJS) -o $(TARGET)

# --- Specific Build Targets ---

# Default target: builds the main executable
all: $(TARGET)

# Clean up generated files
clean:
	@rm -f $(OBJS) $(TARGET) $(OBJS:.o=.d) 2>/dev/null

