#!/usr/bin/make -f

.PHONY: default
default: build

.PHONY: run
run: build/src/bonicpp
	./build/src/bonicpp

build/src/bonicpp: build

.PHONY: build
build: configure
	cmake --build build

.PHONY: configure
configure: build/Makefile

build/Makefile: CMakeLists.txt
	cmake -B build
