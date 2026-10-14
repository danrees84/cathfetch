CXX ?= g++
CPPFLAGS += -Isrc
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
ENGINE_SOURCES := $(filter-out src/main.cpp src/terminal_display.cpp src/terminal_format.cpp,$(wildcard src/*.cpp))
ENGINE_OBJECTS := $(ENGINE_SOURCES:src/%.cpp=build/%.o)
TESTS := $(patsubst tests/%.cpp,build/%,$(wildcard tests/*_test.cpp))
.PHONY: all test clean
all: build/cathfetch build/calendar_year
build:
	mkdir -p build
build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
build/cathfetch: build/main.o build/terminal_display.o build/terminal_format.o $(ENGINE_OBJECTS)
	$(CXX) $(CXXFLAGS) $^ -o $@
build/calendar_year: tools/calendar_year.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
build/%_test: tests/%_test.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
test: $(TESTS) all
	@set -e; for test in $(TESTS); do $$test; done
	@sh tests/cli_test.sh
clean:
	rm -rf build
-include $(wildcard build/*.d)
