SHELL:=/usr/bin/bash

default: build test

##############################################################################
#
# Build
#
##############################################################################

# Recreate Makefiles when CMakeLists.txt changes
./build/debug/Makefile: CMakeLists.txt
	@mkdir -p ./build/debug
	@mkdir -p ./build/release
	@cd build/debug && cmake -D CMAKE_BUILD_TYPE=Debug ../..
	@cd build/release && cmake -D CMAKE_BUILD_TYPE=Release ../..

.PHONY: build # Build all targets
build: ./build/debug/Makefile
	@cd build/debug && make -j
	@cd build/release && make -j

.PHONY: clean # Remove all build dependencies
clean:
	@rm -rf build

##############################################################################
#
# Test
#
##############################################################################

.PHONY: unit_test
unit_test: BUILD=debug
unit_test:
	@parallel --jobs 24 --halt now,fail=1 "echo {} && {}" ::: build/$(BUILD)/test_*

.PHONY: test # Run tests
test:
	@echo "Testing..."
	@$(MAKE) --no-print-directory unit_test BUILD=debug
	@$(MAKE) --no-print-directory unit_test BUILD=release

##############################################################################
#
# Classify and score
#
##############################################################################

INPUT=./data/remote/latest/*.csv

.PHONY: classify # Run classifier
classify: BUILD=debug
classify: OO_PARAMS="--verbose"
classify: MAX_FILES=1000
classify: build
	@mkdir -p predictions
	@ls -1 $(INPUT) \
		| head -$(MAX_FILES) \
		| parallel --verbose --lb --jobs=16 --halt now,fail=1 \
		"build/$(BUILD)/classify $(OO_PARAMS) < {} > predictions/{/.}_classified.csv"

.PHONY: score # Get scores for OO++
score: build
	@./scripts/get_oopp_scores.sh
	@cat ./micro_scores_no_surface.txt
	@cat ./micro_scores_all.txt

.PHONY: search # Search OO parameter space
search: build
	@python ./scripts/generate_search_commands.py --build=release

##############################################################################
#
# Get help by running
#
#     $ make help
#
##############################################################################
.PHONY: help # Generate list of targets with descriptions
help:
	@grep '^.PHONY: .* #' Makefile | sed 's/\.PHONY: \(.*\) # \(.*\)/\1	\2/' | expand -t25
