#ifndef __WORKER_H__
#define __WORKER_H__

#include <stdio.h>
#include "structs.h"
#include "bits.h"

/* The Worker is incharge of all assembler actions.
 * he calls for first and second passes and bilds the outpot files. he also implements usefull functions for the first and second pass. he works hard! */

#define ADDRESS_START       100
#define CODE_SIZE           256 /* 256 in words */
#define MAX_LABEL_LENGTH    30
#define MAX_BUFFER          80 /* TODO: to update the number */
#define NUMBER_OF_REGISTERS 8
#define ZERO_BIT            (1 << PSW_BIT_ZERO)
#define COMMENT   ';'

typedef enum {
    DIRECTIVE_ENTRY = 1,
    DIRECTIVE_EXTERN,
    DIRECTIVE_DATA,
    DIRECTIVE_STRING,
    DIRECTIVE_STRUCT,
} DIRECTIVES_TYPES;

typedef enum {
    OPCODE_MOV = 0,
    OPCODE_CMP,
    OPCODE_ADD,
    OPCODE_SUB,
    OPCODE_NOT,
    OPCODE_CLR,
    OPCODE_LEA,
    OPCODE_INC,
    OPCODE_DEC,
    OPCODE_JMP,
    OPCODE_BNE,
    OPCODE_RED,
    OPCODE_PRN,
    OPCODE_JSR,
    OPCODE_RTS,
    OPCODE_STOP
} COMMANDS_OPCODES;

typedef enum {
    ERR_SUCCESS = 0,
    ERR_NO_FILE,
    ERR_COMPILATION
} ASSEMBLING_ERRORS;


typedef enum {
    BOTH_LEFT = 0,
    SRC_LEFT_DEST_RIGHT,
    SRC_RIGHT_DEST_LEFT,
    BOTH_RIGHT
} COMB_OPTION;

typedef enum {
    ABSOLUTE_TYPE = 0,
    EXTERNAL_TYPE,
    RELOCATABLE_TYPE,
} ENCODING_TYPE;

/* Main function of the assembler, call this upon a readable .as file to start compiling it. Assembles a file using first and seocond pass. */
int assemble(char *filename);

/* Return TRUE if line is .data/.extern/.entry declaration */
Number is_compiler_directive(char *arg);

/* Handles compiler directives (.data/.string) and checks for errors */
Number get_compiler_directive_args_len(char *arg, Number type);

/* Return TRUE if the line is empty or if it's a comment */
BOOL is_useless_line(char *line);

/* Creating outpot files */
void create_externals_file(char *filename);

void create_entries_file(char *filename);

void create_object_file(char *filename);

#endif /* __WORKER_H__ */
