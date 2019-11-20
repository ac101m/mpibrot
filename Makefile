# Release targets
OUTPUT_DIR := bin
CLIENT_RELEASE_TARGET ?= $(OUTPUT_DIR)/client
CLIENT_DEBUG_TARGET ?= $(OUTPUT_DIR)/client-debug
SERVER_RELEASE_TARGET ?= $(OUTPUT_DIR)/server
SERVER_DEBUG_TARGET ?= $(OUTPUT_DIR)/server-debug
TEST_TARGET ?= $(OUTPUT_DIR)/test
TEST_TARGET_MULTINODE ?= $(OUTPUT_DIR)/test-multinode

# Directory controls
OBJ_DIR ?= build
SRC_DIR ?= src
INC_DIRS ?= src include
RESOURCE_DIR ?= data

#====[COMPILER BEHAVIOUR]=====================================================#
CXX := g++
MPICXX := mpicxx
INC_FLAGS ?= -Iinclude -Isrc
COMMON_FLAGS ?= -MMD -MP -m64 -std=c++14 -Wall
DEBUG_FLAGS ?= $(COMMON_FLAGS) -g
RELEASE_FLAGS ?= $(COMMON_FLAGS) -O3
COMMON_LD_FLAGS := -loptparse -l:libboost_system.a -static-libstdc++ -pthread
TEST_LD_FLAGS := $(COMMON_LD_FLAGS)
CLIENT_LD_FLAGS := $(COMMON_LD_FLAGS) -lgltools -lGLEW -lglfw -lGL
SERVER_LD_FLAGS := $(COMMON_LD_FLAGS)

#====[SOURCE AND OBJECT ENUMERATION]==========================================#

# Command to locate non main sources
FIND_NON_MAIN_SRCS := find $(SRC_DIRS) -mindepth 3 -name *.cpp

# Basic test sources
TEST_SRCS := $(shell $(FIND_NON_MAIN_SRCS) | grep -v /draw/ | grep -v /test_multinode/) src/test_main.cpp
TEST_OBJS := $(TEST_SRCS:%=$(OBJ_DIR)/test/%.o)

# Multinode test sources
TEST_SRCS_MULTINODE := $(shell $(FIND_NON_MAIN_SRCS) | grep -v /draw/ | grep -v /test/) src/test_multinode_main.cpp
TEST_OBJS_MULTINODE := $(TEST_SRCS_MULTINODE:%=$(OBJ_DIR)/test_multinode/%.o)

# Client sources and objects (nothing from compute directory)
CLIENT_SRCS := $(shell $(FIND_NON_MAIN_SRCS) | grep -v /compute/ | grep -v /test/ | grep -v /test_multinode/ | grep -v /mpi/) src/client_main.cpp
CLIENT_DEBUG_OBJS := $(CLIENT_SRCS:%=$(OBJ_DIR)/client-debug/%.o)
CLIENT_RELEASE_OBJS := $(CLIENT_SRCS:%=$(OBJ_DIR)/client-release/%.o)

# Server sources and objects (nothing from draw directory)
SERVER_SRCS := $(shell $(FIND_NON_MAIN_SRCS) | grep -v /draw/ | grep -v /test/ | grep -v /test_multinode/) src/server_main.cpp
SERVER_DEBUG_OBJS := $(SERVER_SRCS:%=$(OBJ_DIR)/server-debug/%.o)
SERVER_RELEASE_OBJS := $(SERVER_SRCS:%=$(OBJ_DIR)/server-release/%.o)

# Header dependencies (all of them)
SERVER_DEPS := $(SERVER_DEBUG_OBJS:.o=.d) $(SERVER_RELEASE_OBJS:.o=.d)
CLIENT_DEPS := $(CLIENT_DEBUG_OBJS:.o=.d) $(CLIENT_RELEASE_OBJS:.o=.d)
TEST_DEPS := $(TEST_OBJS:.o=.d) $(TEST_OBJS_MULTINODE:.o=.d)

#====[TEST OBJECT COMPILATION]================================================#
# Single node tests, use mpicxx to avoid errors caused by mpi header inclusion
$(OBJ_DIR)/test/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

# Multi node tests
$(OBJ_DIR)/test_multinode/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

#====[CLIENT OBJECT COMPILATION]==============================================#
# Debug
$(OBJ_DIR)/client-debug/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

# Release
$(OBJ_DIR)/client-release/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(CXX) $(RELEASE_FLAGS) $(INC_FLAGS) -c $< -o $@

#====[SERVER OBJECT COMPILATION]==============================================#
# Debug
$(OBJ_DIR)/server-debug/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(DEBUG_FLAGS) $(INC_FLAGS) -c $< -o $@

# Release
$(OBJ_DIR)/server-release/%.cpp.o: %.cpp
	@$(MKDIR_P) $(dir $@)
	$(MPICXX) $(RELEASE_FLAGS) $(INC_FLAGS) -c $< -o $@

#====[BUILD TARGETS]==========================================================#
# Single node test target, use mpicxx to avoid mpi related linker errors
test: $(TEST_OBJS) copy_resources
	@$(MKDIR_P) $(dir $(TEST_TARGET))
	$(MPICXX) $(TEST_OBJS) -o $(TEST_TARGET) $(TEST_LD_FLAGS)

# Multi node test target
test_multinode: $(TEST_OBJS_MULTINODE) copy_resources
	@$(MKDIR_P) $(dir $(TEST_TARGET_MULTINODE))
	$(MPICXX) $(TEST_OBJS_MULTINODE) -o $(TEST_TARGET_MULTINODE) $(TEST_LD_FLAGS)

# Client debug target
client_debug: $(CLIENT_DEBUG_OBJS) copy_resources
	@$(MKDIR_P) $(dir $(CLIENT_DEBUG_TARGET))
	$(CXX) $(CLIENT_DEBUG_OBJS) -o $(CLIENT_DEBUG_TARGET) $(CLIENT_LD_FLAGS)

# Client release target
client_release: $(CLIENT_RELEASE_OBJS) copy_resources
	@$(MKDIR_P) $(dir $(CLIENT_RELEASE_TARGET))
	$(CXX) $(CLIENT_RELEASE_OBJS) -o $(CLIENT_RELEASE_TARGET) $(CLIENT_LD_FLAGS)

# Server debug target
server_debug: $(SERVER_DEBUG_OBJS) copy_resources
	@$(MKDIR_P) $(dir $(SERVER_DEBUG_TARGET))
	$(MPICXX) $(SERVER_DEBUG_OBJS) -o $(SERVER_DEBUG_TARGET) $(SERVER_LD_FLAGS)

# Server release target
server_release: $(SERVER_RELEASE_OBJS) copy_resources
	@$(MKDIR_P) $(dir $(SERVER_RELEASE_TARGET))
	$(MPICXX) $(SERVER_RELEASE_OBJS) -o $(SERVER_RELEASE_TARGET) $(SERVER_LD_FLAGS)

# Collect all resource files in the bin directories
copy_resources:
	@$(MKDIR_P) $(OUTPUT_DIR)
	cp -r $(RESOURCE_DIR) $(OUTPUT_DIR)

# Agregate build targets
release: server_release client_release
debug: server_debug client_debug
all: release debug

# Clean, be careful with this
.PHONY: clean
clean:
	@$(RM) -rv $(OBJ_DIR)

# Include dependencies
-include $(SERVER_DEPS) $(CLIENT_DEPS) $(TEST_DEPS)

# Make directory
MKDIR_P ?= mkdir -p
