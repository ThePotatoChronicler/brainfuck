.PHONY: build
build: compile_flags.txt
	gcc main.c -o brainfuck -O2 `cat compile_flags.txt`

debug: compile_flags.txt
	gcc main.c -o brainfuck -Og -g -ggdb `cat compile_flags.txt`

.PHONY: clean
clean:
	rm -f compile_flags.txt

compile_flags.txt:
	./configure std
