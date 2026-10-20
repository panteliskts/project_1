CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CXXFLAGS += -MMD -MP -pthread

CORE_SRC := $(wildcard src/core/*.cpp)
CORE_OBJ := $(CORE_SRC:src/%.cpp=build/%.o)

.PHONY: all clean

all: $(CORE_OBJ)

build/core/%.o: src/core/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf build

-include $(wildcard build/*/*.d)
