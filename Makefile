# SPDX-License-Identifier: GPL-3.0-or-later
# Copyright (C) 2026 Daniel Rees
# See LICENSE for licence terms and the disclaimer of warranty.

CXX ?= g++
CPPFLAGS += -Isrc
CXXFLAGS ?= -std=c++20 -O2 -Wall -Wextra -Wpedantic
# The reusable engine excludes main and terminal rendering. Both the app and
# regression tests link these same object files.
ENGINE_SOURCES := $(filter-out src/main.cpp src/terminal_display.cpp src/terminal_format.cpp,$(wildcard src/*.cpp))
ENGINE_OBJECTS := $(ENGINE_SOURCES:src/%.cpp=build/%.o)
TESTS := $(patsubst tests/%.cpp,build/%,$(wildcard tests/*_test.cpp))
PREFIX ?= /usr/local
DESTDIR ?=
.PHONY: all test clean summary-review install
all: build/cathfetch build/calendar_year build/lectionary_year build/summary_year build/en_dra.json
# Keep source/provenance files alongside installed data. DESTDIR permits staged
# packaging without writing to the running system.
install: build/cathfetch build/en_dra.json LICENSE LICENSING.md licenses/nlohmann-json-MIT.txt
	install -d "$(DESTDIR)$(PREFIX)/bin" "$(DESTDIR)$(PREFIX)/share/cathfetch"
	install -m 755 build/cathfetch "$(DESTDIR)$(PREFIX)/bin/cathfetch"
	cp -R data/. "$(DESTDIR)$(PREFIX)/share/cathfetch/"
	install -m 644 LICENSE LICENSING.md "$(DESTDIR)$(PREFIX)/share/cathfetch/"
	install -d "$(DESTDIR)$(PREFIX)/share/cathfetch/licenses"
	install -m 644 licenses/nlohmann-json-MIT.txt "$(DESTDIR)$(PREFIX)/share/cathfetch/licenses/"
	install -m 644 build/en_dra.json "$(DESTDIR)$(PREFIX)/share/cathfetch/bibles/en_dra.json"
build/summary_year: tools/summary_year.cpp $(ENGINE_OBJECTS) | build/en_dra.json
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
build/summary_catalogue: tools/summary_catalogue.cpp $(ENGINE_OBJECTS) | build/en_dra.json
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
summary-review: build/summary_catalogue build/summary_year
	python3 tools/summary_review.py export
build:
	mkdir -p build
# -MMD/-MP record header dependencies for later incremental builds.
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
build/scripture_summary_test: | build/en_dra.json
build/%_test: tests/%_test.cpp $(ENGINE_OBJECTS)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $^ -o $@
# Stop at the first failing suite so a partial run is not reported as success.
test: $(TESTS) all
	@set -e; for test in $(TESTS); do $$test; done
	@sh tests/cli_test.sh
	@sh tests/lectionary_cli_test.sh
	@python3 tests/universalis_audit_test.py
	@python3 tests/bible_mapping_data_test.py
	@python3 tests/current_lectionary_audit_test.py
	@sh tests/full_verses_cli_test.sh
	@python3 tests/summary_cli_test.py
	@python3 tests/data_path_cli_test.py
	@$(MAKE) --no-print-directory build/summary_catalogue
	@python3 tools/summary_review.py check
clean:
	rm -rf build
-include $(wildcard build/*.d)
