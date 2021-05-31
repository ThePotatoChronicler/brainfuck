WARNING: This thing is full of bugs, so yknow, don't depend on it.
         there are still programs that will crash the interpreter.
         Please report those, or don't.

Implementation notes:
    The memory array is infinite, and will take more and more memory
    if it's required, and will only free it at the end of the program.

Compilation options:
    // You don't actually have to write these yourself,
    // that's what the configure script is for.
    // (If I make it, that is)

    BF_MEMORY_BELOW_ZERO:
        Allows going below 0 in the Brainfuck memory array.
        Default behaviour is throwing an error.

    BF_ERR_UNK_INST:
        The program will error when encountering unknown symbols.
        The exception is a newline.

    BF_ALT_INPUT:
        Stops the input from being echoed,
        and the program will go by character.

    BF_CELL_SIZE_[BITS]:
        Changes the size of the memory cells to the
        specified amount of bits.
        Default is 8.

    BF_CELL_0_RETURN:
        The program's return code will be whatever
        is in the 0th cell of the memory array.
        The behaviour is undefined when BF_MEMORY_BELOW_ZERO
        is enabled.
