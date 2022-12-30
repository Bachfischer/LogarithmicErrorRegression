FOLDERS:=bin/ results/
BENCH:=bin/benchmark

OPTIONS:=-march=native -O3 -fopenmp -std=c++17
INCLUDE:=$(wildcard src/*) $(wildcard src/helpers/*)

.PHONY : all

all:$(FOLDERS) $(BENCH)

clean:
	rm bin/*

reset:
	rm results/*

$(BENCH) : $(INCLUDE) benchmark.cpp
	g++-11 -o $(BENCH) benchmark.cpp $(OPTIONS) -I src
