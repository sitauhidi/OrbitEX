# Makefile for OrbitSI C++ Project

# --- Variables ---

# Compiler to use
CXX = g++

# Base compiler flags for all files
BASE_CXXFLAGS = -std=c++17 -O3 -g -fopenmp

# Stricter flags for our application code
APP_CXXFLAGS = $(BASE_CXXFLAGS) -Wall

# --- Directories and Files ---

# The final executable name
EXECUTABLE = orbitsi

# Directory for all build output
BUILD_DIR = build

# Source and Include directories
SRC_DIR = src
INCLUDE_DIR = include
ESCAPE_DIR = extern/Escape

# Include paths: Tell the compiler where to find header files.
# -I$(INCLUDE_DIR): Look for our project's headers in the 'include' directory.
# -isystem extern: Look for library headers in 'extern', suppressing their warnings.
INCLUDES = -I$(INCLUDE_DIR) -isystem extern

# Find all C++ source files
SRC_SOURCES    = $(wildcard $(SRC_DIR)/*.cpp)
ESCAPE_SOURCES = $(wildcard $(ESCAPE_DIR)/*.cpp)

# Generate object file paths
SRC_OBJS    = $(patsubst $(SRC_DIR)/%.cpp, $(BUILD_DIR)/src/%.o, $(SRC_SOURCES))
ESCAPE_OBJS = $(patsubst $(ESCAPE_DIR)/%.cpp, $(BUILD_DIR)/escape/%.o, $(ESCAPE_SOURCES))

# Combine all object files
ALL_OBJS = $(SRC_OBJS) $(ESCAPE_OBJS)

# The full path to the final target executable
TARGET = $(BUILD_DIR)/$(EXECUTABLE)


# --- Build Rules ---

# Default target: build the executable
all: $(TARGET)

# Rule to link the final executable
$(TARGET): $(ALL_OBJS)
	@echo "Linking executable..."
	@mkdir -p $(@D)
	$(CXX) $(APP_CXXFLAGS) -o $@ $^
	@echo "Build successful! Executable is at: $(TARGET)"

# Rule to compile our application source files with strict warnings.
# This rule now also depends on the header files in the include directory.
$(BUILD_DIR)/src/%.o: $(SRC_DIR)/%.cpp $(wildcard $(INCLUDE_DIR)/*.h)
	@echo "Compiling [app]   : $<"
	@mkdir -p $(@D)
	$(CXX) $(APP_CXXFLAGS) $(INCLUDES) -c $< -o $@

# Rule to compile the external library files with warnings suppressed
$(BUILD_DIR)/escape/%.o: $(ESCAPE_DIR)/%.cpp
	@echo "Compiling [escape]: $<"
	@mkdir -p $(@D)
	$(CXX) $(BASE_CXXFLAGS) -w $(INCLUDES) -c $< -o $@


# --- Utility Rules ---

# Rule to run the test suite for graphlet size 4
test4: all
	@echo "\n--- Running Test Suite (pytest, graphlet-size=4) ---"
	GRAPHLET_SIZE=4 pytest test/

# Rule to run the test suite for graphlet size 5
test5: all
	@echo "\n--- Running Test Suite (pytest, graphlet-size=5) ---"
	GRAPHLET_SIZE=5 pytest test/

# Main test rule that runs all test configurations
test: test4 test5
	@echo "\n--- All Test Suites Passed ---"

# Rule to clean up all build artifacts
clean:
	@echo "Cleaning up build artifacts..."
	rm -rf $(BUILD_DIR)

# Phony targets are not files
.PHONY: all clean test

