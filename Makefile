CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CXXFLAGS += -MMD -MP -pthread -Iinclude

# OpenCV 4.x ships opencv4.pc, OpenCV 5.x ships opencv5.pc. Its headers are
# included as system headers so -pedantic only warns about our own code.
OPENCV_PC     := $(shell pkg-config --exists opencv4 && echo opencv4 || echo opencv5)
OPENCV_CFLAGS := $(patsubst -I%,-isystem %,$(shell pkg-config --cflags $(OPENCV_PC)))
OPENCV_LIBS   := $(shell pkg-config --libs $(OPENCV_PC))

CORE_SRC := $(wildcard src/core/*/*.cpp)
PIPE_SRC := $(wildcard src/pipeline/*/*.cpp) src/main.cpp
CORE_OBJ := $(CORE_SRC:src/%.cpp=build/%.o)
PIPE_OBJ := $(PIPE_SRC:src/%.cpp=build/%.o)

.PHONY: all test clean

all: search

search: $(CORE_OBJ) $(PIPE_OBJ)
	$(CXX) $(CXXFLAGS) -o $@ $^ $(OPENCV_LIBS)

# The core has no OpenCV dependency, so its tests build without it.
test_core: $(CORE_OBJ) build/tests/test_core.o
	$(CXX) $(CXXFLAGS) -o $@ $^

test: test_core
	./test_core

build/core/%.o: src/core/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/tests/%.o: tests/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

build/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) $(OPENCV_CFLAGS) -c $< -o $@

clean:
	rm -rf build search test_core

-include $(wildcard build/*.d build/*/*.d build/*/*/*.d)