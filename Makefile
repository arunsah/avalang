.DEFAULT_GOAL := build

BUILD_DIR ?= build
REVIEW_BUILD_DIR ?= build-review

CMAKE := cmake
CTEST := ctest
CLANG_FORMAT := clang-format

C_FILES := $(wildcard src/*.c src/*.h tests/*.c tests/*.h)

.PHONY: configure build test format format-check check review clean

configure:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Debug

build: configure
	$(CMAKE) --build $(BUILD_DIR)

test: build
	$(CTEST) --test-dir $(BUILD_DIR) --output-on-failure

format:
	$(CLANG_FORMAT) -i $(C_FILES)

format-check:
	$(CLANG_FORMAT) --dry-run --Werror $(C_FILES)

check: test format-check

review:
	$(CMAKE) -S . -B $(REVIEW_BUILD_DIR) \
		-DCMAKE_BUILD_TYPE=Debug \
		-DCMAKE_C_FLAGS='-Werror -fsanitize=address,undefined -fno-sanitize-recover=undefined -fno-omit-frame-pointer'
	$(CMAKE) --build $(REVIEW_BUILD_DIR)
	$(CTEST) --test-dir $(REVIEW_BUILD_DIR) --output-on-failure
	$(CLANG_FORMAT) --dry-run --Werror $(C_FILES)

clean:
	$(CMAKE) -E remove_directory $(BUILD_DIR)
	$(CMAKE) -E remove_directory $(REVIEW_BUILD_DIR)
