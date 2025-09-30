.PHONEY: build, run

build: program

run: program
	./program $(file)

program: main.c libs/uselibpng.c
	clang -ggdb -O0 -I./libs main.c libs/uselibpng.c -lpng -o program