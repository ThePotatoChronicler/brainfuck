// Name: Brainfuck Interpreter
// Version: Terrible
// Author: The Potato Chronicler
// License: Bruh

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#ifdef BF_ALT_INPUT
    #include <termios.h>
#endif

#include <stdbool.h>

#define READAMOUNT 100
// How much to read at once
// no idea why I added the option
// to change this
//
// TODO: Make this a const variable, since this is dumb
//       this might not be possible, since it needs
//       to be a compile-time constant, but meh,
//       what do I care.

// Definitions have been moved out of main and put
// here, so they can be accessed by signal handler(s).
// TODO: Figure out a better way to do this than
//       make part of the variables global . _.
FILE* source = NULL;
char buffer[READAMOUNT];
#ifdef BF_ALT_INPUT
    struct termios termsave, // Terminal attributes save
                   termtemp; // Temporary termios object to change attributes of
    bool termset = false; // Remembering if terminal state was saved into termsave
#endif

// Brainfuck's memory array
#if defined(BF_CELL_SIZE_16)
    unsigned short int* bmemory = NULL;
#elif defined(BF_CELL_SIZE_32)
    unsigned long int* bmemory = NULL;
#elif defined(BF_CELL_SIZE_64)
    unsigned long long int* bmemory = NULL;
#else
    unsigned char* bmemory = NULL;
#endif

static void sigint_handle (int sig) {
    if (source != NULL) {
        fclose(source);
    }
    if (bmemory != NULL) {
        free(bmemory);
    }
    #ifdef BF_ALT_INPUT
    if (termset) {
        tcsetattr(fileno(stdin), TCSANOW, &termsave);
    }
    #endif
    exit(128 + sig);
}

int main(int argc, char** argv) {

    long int i; // Used for looping
    int retnum = 0; // Somewhere to save the return number into
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
    const char bcellsize = sizeof(*bmemory);

    signal(SIGINT, sigint_handle);

    if (argc < 2) {
        fprintf(stderr, "Atleast one argument is expected >:[\n");
        retnum = EXIT_FAILURE;
        goto ON_ERROR;
    }
    source = fopen(argv[1], "rb");
    if (source == NULL) {
        fprintf(stderr, "An error has occured while opening file, perhaps check the filename\n");
        retnum = EXIT_FAILURE;
        goto ON_ERROR;
    }

    #ifdef BF_ALT_INPUT
    tcgetattr(fileno(stdin), &termsave); // Saving terminal state so we can recover it later
    termset = true;
    termtemp = termsave;
    termtemp.c_lflag &= ~(ECHO | ECHONL | ICANON);
    // Disables:
    // ECHO - No echoing
    // ECHONL - Do not echo newlines either
    // ICANON - Don't go line by line, but character by character
    // Check termios(3) at your own risk

    tcsetattr(fileno(stdin), TCSANOW, &termtemp);
    #endif

    bmemory = calloc(bsize, bcellsize); // Zero-initialized block of memory
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
                        #ifndef BF_MEMORY_BELOW_ZERO
                            fprintf(stderr, "Attempt to go below 0 of the memory array!\n");
                            retnum = EXIT_FAILURE;
                            goto ON_ERROR;
                        #else
                            bmemory = realloc(bmemory, bsize + bnewsize);
                            for (i = 0; i != bsize; ++i) {
                                bmemory[i] = bmemory[i + bnewsize];
                            }
                            for (i = 0; i != bnewsize; ++i) {
                                bmemory[i] = 0;
                            }
                            bsize += bnewsize;
                        #endif
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
                                retnum = EXIT_FAILURE;
                                goto ON_ERROR;
                            }
                            aread = fread(buffer, 1, READAMOUNT, source);
                            apoint = 0;
                        }
                        EXIT_WHILE:;
                    }
                    break;
                case ']':
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
                                retnum = EXIT_FAILURE;
                                goto ON_ERROR;
                            }

                            fseek(source, (signed)-(aread + READAMOUNT), SEEK_CUR);
                            aread = fread(buffer, 1, READAMOUNT, source);
                            apoint = aread - 1;
                        }
                        EXIT_WHILE2:;
                    }
                    break;
                #ifdef BF_ERR_UNK_INST
                case '\n':
                    continue;
                default:
                    if (buffer[apoint] == '\n') {
                        fprintf(stderr, "Unknown instruction '\n'(10) at position %llu\n", charat + 1);
                    }
                    else {
                        fprintf(stderr, "Unknown instruction '%c'(%u) at position %llu\n", buffer[apoint], buffer[apoint], charat + 1);
                    }
                    retnum = EXIT_FAILURE;
                    goto ON_ERROR;
                #endif
            }
        }

        if (aread != READAMOUNT) {
            #ifdef BF_CELL_0_RETURN
                retnum = *bmemory;
            #endif
            break;
        }
    }

    ON_ERROR:;
    if (source != NULL) {
        fclose(source);
    }
    if (bmemory != NULL) {
        free(bmemory);
    }
    #ifdef BF_ALT_INPUT
    if (termset) {
        tcsetattr(fileno(stdin), TCSANOW, &termsave);
    }
    // Resets terminal to initial state
    #endif

    return retnum; // Returns first of the brainfuck array, or EXIT_FAILURE.
}
