#!/usr/bin/make -f

build_dir = build

.PHONY: default
default: build

.PHONY: run
run: "$(build_dir)/src/bonicpp"
	"./$(build_dir)/src/bonicpp"

"$(build_dir)/src/bonicpp": build

.PHONY: build
build: configure
	cmake --build "$(build_dir)"

.PHONY: clean
clean:
	rm -dfr "$(build_dir)"

.PHONY: configure
configure: "$(build_dir)/Makefile"

"$(build_dir)/Makefile": CMakeLists.txt
	cmake -B "$(build_dir)" --preset all
