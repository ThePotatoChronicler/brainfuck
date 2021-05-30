build:
	gcc main.c -o brainfuck -O2

debug:
	gcc main.c -o brainfuck -Og -g -ggdb
