# Release targets
TARGET_RELEASE ?= bin/client
TARGET_DEBUG ?= bin/client-debug

# Directory controls
OBJ_DIR ?= build
SRC_DIR ?= src
INC_DIRS ?= src include
RESOURCE_DIR ?= data

# Compiler configuration
CXX := g++
FLAGS_BASE ?= -MMD -MP -m64 -std=c++14 -Wall
FLAGS_RELEASE ?= $(FLAGS_BASE) -O3
FLAGS_DEBUG ?= $(FLAGS_BASE) -g
INC_FLAGS ?= -Iinclude -Isrc
LD_FLAGS ?= -loptparse -lgltools -lGLEW -lglfw -lGL

# Enumerate sources
SRCS := $(shell find $(SRC_DIR) -name *.cpp)
OBJS_RELEASE := $(SRCS:%=$(OBJ_DIR)/release/%.o)
OBJS_DEBUG := $(SRCS:%=$(OBJ_DIR)/debug/%.o)
DEPS := $(OBJS_DEBUG:.o=.d) $(OBJS_RELEASE:.o=.d)

# Release object compilation
$(OBJ_DIR)/release/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(FLAGS_RELEASE) $(INC_FLAGS) -c $< -o $@

# Debug object compilation
$(OBJ_DIR)/debug/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(FLAGS_DEBUG) $(INC_FLAGS) -c $< -o $@

# Release target
release: copy_resources $(OBJS_RELEASE)
	@$(MKDIR_P) $(dir $(TARGET_RELEASE))
	$(CXX) $(OBJS_RELEASE) -o $(TARGET_RELEASE) $(LD_FLAGS)

# Release target
debug: copy_resources $(OBJS_DEBUG)
	@$(MKDIR_P) $(dir $(TARGET_DEBUG))
	$(CXX) $(OBJS_DEBUG) -o $(TARGET_DEBUG) $(LD_FLAGS)

# Collect all resource files in the bin directors
copy_resources:
	@$(MKDIR_P) $(dir $(TARGET_RELEASE))
	cp -r $(RESOURCE_DIR) $(dir $(TARGET_RELEASE))

# Build everything
all: release debug

# Clean, be careful with this
.PHONY: clean
clean:
	@$(RM) -rv $(OBJ_DIR)

# Include dependencies
-include $(DEPS)

# Make directory
MKDIR_P ?= mkdir -p
