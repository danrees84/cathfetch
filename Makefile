CXX ?= g++
CPPFLAGS += -Isrc
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
ENGINE_SOURCES := $(filter-out src/main.cpp src/terminal_display.cpp src/terminal_format.cpp,$(wildcard src/*.cpp))
ENGINE_OBJECTS := $(ENGINE_SOURCES:src/%.cpp=build/%.o)
TESTS := $(patsubst tests/%.cpp,build/%,$(wildcard tests/*_test.cpp))
.PHONY: all test clean
all: build/cathfetch build/calendar_year build/lectionary_year build/en_dra.json
build:
	mkdir -p build
build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -MMD -MP -c $< -o $@
build/en_dra.json: data/bibles/en_dra.xml data/bibles/en_dra_metadata.json data/bibles/en_dra_versification.json tools/bible_data.py | build
	python3 tools/bible_data.py index data/bibles/en_dra.xml data/bibles/en_dra_metadata.json $@
build/cathfetch: build/main.o build/terminal_display.o build/terminal_format.o $(ENGINE_OBJECTS) | build/en_dra.json
	$(CXX) $(CXXFLAGS) $^ -o $@
build/calendar_year: tools/calendar_year.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
build/lectionary_year: tools/lectionary_year.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
build/terminal_render_test: tests/terminal_render_test.cpp build/terminal_display.o build/terminal_format.o $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
build/scripture_test: | build/en_dra.json
build/%_test: tests/%_test.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
test: $(TESTS) all
	@set -e; for test in $(TESTS); do $$test; done
	@sh tests/cli_test.sh
	@sh tests/lectionary_cli_test.sh
	@python3 tests/universalis_audit_test.py
	@python3 tests/bible_mapping_data_test.py
	@sh tests/full_verses_cli_test.sh
clean:
	rm -rf build
-include $(wildcard build/*.d)
