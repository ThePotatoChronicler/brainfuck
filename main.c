// Name: Brainfuck Interpreter
// Version: Terrible
// Author: The Potato Chronicler
// License: Bruh

#include <stdio.h>
#include <stdlib.h>
#include <termios.h>
#include <stdbool.h>

#define READAMOUNT 100
// How much to read at once
// no idea why I added the option
// to change this
//
// TODO: Make this a const variable, since this is dumb

int main(int argc, char** argv) {

    FILE* source;
    char buffer[READAMOUNT];
    struct termios termsave, // Terminal attributes save
                   termtemp; // Temporary termios object to change attributes of
    int retnum; // Somewhere to save the return number into
                // before the program memory is freed and it's lost
    unsigned long long int charat = 0; // Amount of characters gone through.
                                       // This should not be used as a way
                                       // to track progress of any kind
    unsigned int bdepth; // Bracket depth, used for brackets (obviously)
    unsigned int aread  = 0, // Amount read
                 apoint = 0; // Point in the read buffer
    unsigned long long int bpoint = 1,   // Brainfuck memory pointer
                           bsize  = 100; // Brainfuck memory array size
    const long long int bnewsize = 100; // How much to increase memory size by when needed
    unsigned char* bmemory; // Brainfuck's memory array
    bmemory = calloc(bsize, 1); // Zero-initialized block of memory

    if (argc < 2) {
        fprintf(stderr, "Atleast one argument is expected >:[\n");
        return EXIT_FAILURE;
    }
    source = fopen(argv[1], "rb");
    if (source == NULL) {
        fprintf(stderr, "An error has occured while opening file, perhaps check the filename\n");
        return EXIT_FAILURE;
    }

    tcgetattr(fileno(stdin), &termsave); // Saving terminal state so we can recover it later
    termtemp = termsave;
    termtemp.c_lflag &= ~(ECHO | ECHONL | ICANON);
    // Disables:
    // ECHO - No echoing
    // ECHONL - Do not echo newlines either
    // ICANON - Don't go line by line, but character by character
    // Check termios(3) at your own risk

    tcsetattr(fileno(stdin), TCSANOW, &termtemp);

    while (true) {
        aread = fread(buffer, 1, READAMOUNT, source);

        for (apoint = 0; apoint != aread; ++apoint) {
            ++charat;
            switch (buffer[apoint]) {
                case '+':
                    ++bmemory[bpoint];
                    break;
                case '-':
                    --bmemory[bpoint];
                    break;
                case '>':
                    ++bpoint;
                    if (bpoint >= bsize) {
                        bmemory = realloc(bmemory, bsize + bnewsize);
                        bsize += bnewsize;
                    }
                    break;
                case '<':
                    if (bpoint == 0) {
                        fprintf(stderr, "Attempt to go below 0 of the memory array!\n");
                    }
                    --bpoint;
                    break;
                case '.':
                    printf("%c", bmemory[bpoint]);
                    break;
                case ',':
                    bmemory[bpoint] = getchar();
                    break;
                case '[':
                    if (bmemory[bpoint] == 0) {
                        bdepth = 0;
                        while (true) {
                            for (;apoint != aread; ++apoint) {
                                ++charat;
                                bdepth += (buffer[apoint] == '[') - (buffer[apoint] == ']');
                                if (bdepth == 0) { goto EXIT_WHILE; }
                            }

                            if (aread != READAMOUNT) {
                                fprintf(stderr, "A matching closing bracket was never found, exiting program!\n");
                                return EXIT_FAILURE;
                            }
                            aread = fread(buffer, 1, READAMOUNT, source);
                        }
                        EXIT_WHILE:;
                    }
                    break;
                case ']':
                    // Yes, there are two comments saying
                    // to fix this part, but it's 3:40 AM
                    // and I don't have the energy for this.

                    // This part is a mess
                    // reading backwards through a file
                    // is a terrible idea, don't do it.
                    //
                    // TODO: NOT ACTUALLY READING BACKWARDS THROUGH A FILE
                    // Maybe logging all of the brackets into an array
                    // would work, since it would make this part faster
                    // and less terrible.

                    if (bmemory[bpoint] != 0) {
                        bdepth = 0;
                        while (true) {
                               // This condition depends on integer (over|under)flow being a thing
                            for (;apoint + 1 != 0; --apoint) {
                                --charat;
                                bdepth += (buffer[apoint] == ']') - (buffer[apoint] == '[');
                                if (bdepth == 0) { goto EXIT_WHILE2; }
                            }

                            if (((ftell(source) - aread) == 0)) {
                                fprintf(stderr, "A matching opening bracket was never found, exiting program!\n");
                                return EXIT_FAILURE;
                            }


                            // This part limits how long a file can be by half,
                            // because a signed int just ain't as thicc as unsigned int.
                            // Also, it will probably cause some random bugs with longer
                            // programs, because of (under/over)flow.
                            // TODO: Implement this thing properly, this is stupid.

                            fseek(source, (signed)-(aread + READAMOUNT), SEEK_CUR);
                            aread = fread(buffer, 1, READAMOUNT, source);
                            apoint = aread - 1;
                        }
                        EXIT_WHILE2:;
                    }
                    break;
                case '\n':
                    continue;
                default:
                    fprintf(stderr, "Unknown instruction '%c'(%u) at position %llu\n", buffer[apoint], buffer[apoint], charat + 1);
                    return EXIT_FAILURE;
            }

        }

        if (aread != READAMOUNT) {
            break;
        }
    }

    fclose(source);
    retnum = *bmemory;
    free(bmemory);

    tcsetattr(fileno(stdin), TCSANOW, &termsave);
    // Resets terminal to initial state

    return retnum; // Returns first of the brainfuck array.
}
