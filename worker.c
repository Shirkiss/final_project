/*
 *  Maman 14
 *  worker.c
 *  Created on: Feb, 2018
 *  Author: Shir Cohen
 *  ID: 200948347
 *  Worker is incharge of all assembler actions. he calls for first and second passes and bilds the outpot files. 
 *  he also implements usefull functions for the first and second pass. he works hard!
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "worker.h"
#include "useful.h"
#include "firstpass.h"
#include "secondpass.h"

/* useful variables */
Number code[CODE_SIZE]; /* CODE_SIZE * sizeof(num) bits. This will hold the code and, in the end, the data too */
char word_type[CODE_SIZE]; /* 'a', 'r', 'e' */
Number data[CODE_SIZE]; /* Store the data until we're done processing instructions */
Number cmdCount = 0;
Number dataCount = 0;
Number line_number = 0;
label *labels; /* Hold labels here */
Number num_of_labels = 0;

Number ext_count = 0;
label *ext_address; /* Remember adress of external labels in the file, here */

extern BOOL was_error;

const directive directives_type[] =
        {
                {".entry",  DIRECTIVE_ENTRY},
                {".extern", DIRECTIVE_EXTERN},
                {".data",   DIRECTIVE_DATA},
                {".string", DIRECTIVE_STRING},
                {".struct", DIRECTIVE_STRUCT},
                {NULL,      0}
        };

/* Creating list of commands based on the  command struct:
 *  char *name;
    Number num_of_arguments;
    Number opcode;
    BOOL legal_source_addressing_methods[ADDRESS_METHOD_NUM];
    BOOL legal_dest_addressing_methods[ADDRESS_METHOD_NUM];
 */
const command commands[] =
        {
                {"mov",  2, OPCODE_MOV,  {TRUE,  TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE,  TRUE}},
                {"cmp",  2, OPCODE_CMP,  {TRUE,  TRUE,  TRUE,  TRUE},  {TRUE,  TRUE,  TRUE,  TRUE}},
                {"add",  2, OPCODE_ADD,  {TRUE,  TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE,  TRUE}},
                {"sub",  2, OPCODE_SUB,  {TRUE,  TRUE,  TRUE,  TRUE},  {FALSE, TRUE,  TRUE,  TRUE}},
                {"not",  1, OPCODE_NOT,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"clr",  1, OPCODE_CLR,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"lea",  2, OPCODE_LEA,  {FALSE, TRUE,  TRUE,  FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"inc",  1, OPCODE_INC,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"dec",  1, OPCODE_DEC,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"jmp",  1, OPCODE_JMP,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"bne",  1, OPCODE_BNE,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"red",  1, OPCODE_RED,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"prn",  1, OPCODE_PRN,  {FALSE, FALSE, FALSE, FALSE}, {TRUE,  TRUE,  TRUE,  TRUE}},
                {"jsr",  1, OPCODE_JSR,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, TRUE,  TRUE,  TRUE}},
                {"rts",  0, OPCODE_RTS,  {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE}},
                {"stop", 0, OPCODE_STOP, {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE}},
                {NULL,   0, 0,           {FALSE, FALSE, FALSE, FALSE}, {FALSE, FALSE, FALSE, FALSE}}
        };


/* Main function of the assembler, call this upon a readable .as file to start compiling it. Assembles a file using first and seocond pass. */
int assemble(char *filename) {
    char *fileplusextension;
    Number compiling = 1;
    FILE *fd;
    /* adding extention .as to the file name */
    fileplusextension = (char *) malloc(strlen(filename) + 4);
    if (!fileplusextension) {
        printf("memory allocation failed");
        exit(0);
    }
    strcpy(fileplusextension, filename);
    strcat(fileplusextension, ".as"); /* add .as to the end of the string */

    /* TODO: to change the line before finishing
     * fd = fopen(fileplusextension, "r"); */
    fd = fopen("C:\\Users\\shir.cohen\\CLionProjects\\final_project\\InputA.as", "r");
    line_number = 0;
    if (fd == NULL)
        return ERR_NO_FILE;
    cmdCount = 0;
    /* calling first and second pass */
    while (compiling) {
        compiling = firstpass_analyze_line(fd);
    }
    if (!was_error) {
        cmdCount = -1;
        fseek(fd, 0L, SEEK_SET);
        do {
            compiling = secondpass_analyze_line(fd);
        } while (compiling);
        /* creating outpot files */
        if (!was_error) {
            create_entries_file(filename);
            create_externals_file(filename);
            create_object_file(filename);
        }
    }
    /* freeing memory */
    while (num_of_labels) {
        free(labels[num_of_labels - 1].name);
        num_of_labels--;
    }
    free(labels);
    while (ext_count) {
        free(ext_address[ext_count - 1].name);
        ext_count--;
    }
    free(ext_address);
    cmdCount = 0;
    dataCount = 0;
    line_number = 0;
    num_of_labels = 0;
    ext_count = 0;
    while (line_number < CODE_SIZE) {
        code[line_number] = 0;
        data[line_number] = 0;
        word_type[line_number] = 0;
        line_number++;
    }
    line_number = 0;
    compiling = (was_error) ? -1 : ERR_SUCCESS;
    was_error = FALSE;
    fclose(fd);
    free(fileplusextension);
    return compiling;
}

/* Return TRUE if line is .data/.extern/.entry declaration */
Number is_compiler_directive(char *line) {
    Number i;
    char arg[MAX_BUFFER];
    if (!line || *line == '\0')
        return FALSE;
    separate_arg(line, arg, MAX_BUFFER);
    /* Search for a match between the word stored in 'arg' and any of the legal compiler directives */
    for (i = 0; directives_type[i].name != '\0'; ++i) {
        if (strncmp(arg, directives_type[i].name, 10) == 0) {
            return directives_type[i].directive_identifier;
        }
    }
    return FALSE;
}

/* Handles compiler directives (.data/.string/.struct) and checks for errors */
Number get_compiler_directive_args_len(char *arg, Number type) {
    Number i = 0, j = 0, end_arg_correct = 0, temp;
    char number[MAX_BUFFER], extra_space[MAX_BUFFER];
    char *next;
    if (type == DIRECTIVE_STRING) {
        if (arg[0] != '"') {
            /* Doesn't start with " */
            print_err(ERROR_MISSING_STRING);
            return -1;
        }
        i = strlen(arg);
        if (i == 1) {
            /* Doesn't end with " */
            print_err(ERROR_MISSING_STRING);
            return -1;
        }
        if (i == 2) {
            /* Empty string */
            print_err(ERROR_MISSING_STRING);
            return -1;
        }
        arg++;
        for (i = 0; arg[i] != '\0' && arg[i] != '\n'; ++i) {
            if (arg[i] == '"') {
                end_arg_correct = 1;
                arg[i] = '\0';
                break;
            }
        }
        if (!end_arg_correct) {
            /* Doesn't end with " */
            print_err(ERROR_MISSING_STRING);
            return -1;
        }
        while (*arg != '\0') {
            data[dataCount++] = *arg++;
        }
        data[dataCount++] = '\0';
        return (i + 1);
    } else if (type == DIRECTIVE_DATA) {
        /* extracts the numbers and check for errors */
        i = 0;
        strcpy(extra_space, arg);
        next = strtok(extra_space, " \t,");
        while (next) {
            ++i;
            strcpy(number, next);

            if (extract_num(number, &temp) == -1) {
                print_err(ERROR_BAD_NUMBER);
                return -1;
            } else {
                data[dataCount++] = temp;
            }
            next = strtok(NULL, " \t,");
        }
        return (i);
    } else if (type == DIRECTIVE_STRUCT) {
        /* extracts the items and check for errors */
        i = 0;
        strcpy(extra_space, arg);
        next = strtok(extra_space, " \t,");
        while (next) {
            if (next[0] != '"') {
                ++i;
                strcpy(number, next);
                if (extract_num(number, &temp) == -1) {
                    print_err(ERROR_BAD_NUMBER);
                    return -1;
                } else {
                    data[dataCount++] = temp;
                }
                next = strtok(NULL, " \t,");
            } else {
                j = strlen(next);
                if (j == 1) {
                    /* Doesn't end with " */
                    print_err(ERROR_MISSING_STRING);
                    return -1;
                }
                if (j == 2) {
                    /* Empty string */
                    print_err(ERROR_MISSING_STRING);
                    return -1;
                }
                next++;
                for (j = 0; next[j] != '\0' && next[j] != '\n'; ++j) {
                    if (next[j] == '"') {
                        end_arg_correct = 1;
                        next[j] = '\0';
                        break;
                    }
                }
                if (!end_arg_correct) {
                    /* Doesn't end with " */
                    print_err(ERROR_MISSING_STRING);
                    return -1;
                }
                while (*next != '\0') {
                    data[dataCount++] = *next++;
                }
                data[dataCount++] = '\0';
                next = strtok(NULL, " \t,");
                i = i + j + 1;
            }
        }
        return (i);
    }
    return -1;
}

/* Return TRUE if the line is empty or have only white characters */
BOOL is_useless_line(char *line) {
    Number i;
    /* If there's no line or it's a comment, return true */
    if (!line || line[0] == COMMENT)
        return TRUE;
    /* goes through all the characters, see if we have a nonwhitespace character */
    for (i = 0; line[i] != '\0' && i < MAX_BUFFER; ++i) {
        if (line[i] != '\t' && line[i] != ' ' && line[i] != '\n')
            return FALSE; /* found a non whitespace character, this is not a useless line */
    }
    return TRUE;
}

/* Creating outpot files */
void create_externals_file(char *filename) {
    char *fileplusextension;
    FILE *fd;
    Number i;
    /* creates the file only if there were externals */
    if (ext_count > 0) {
        fileplusextension = (char *) malloc(strlen(filename) + 5);
        if (!fileplusextension) {
            printf("memory allocation failed");
            exit(0);
        }
        strcpy(fileplusextension, filename);
        strcat(fileplusextension, ".ext");
        fd = fopen(fileplusextension, "w");
        for (i = 0; i < ext_count; ++i) {
            fprintf(fd, "%-31s %2s\n", ext_address[i].name, convertBase(ext_address[i].address));
        }
        fclose(fd);
        free(fileplusextension);
    }
}

void create_entries_file(char *filename) {
    char *fileplusextension;
    FILE *fd;
    Number i;
    int exist = 0;
    for (i = 0; i < num_of_labels; ++i) {
        if (labels[i].type == LABEL_TYPE_ENTRY)
            exist = 1;
    }
    /* creates the file only if there were entries */
    if (exist == 1) {
        fileplusextension = (char *) malloc(strlen(filename) + 5);
        if (!fileplusextension) {
            printf("memory allocation failed");
            exit(0);
        }
        strcpy(fileplusextension, filename);
        strcat(fileplusextension, ".ent");
        fd = fopen(fileplusextension, "w");

        for (i = 0; i < num_of_labels; ++i) {
            if (labels[i].type == LABEL_TYPE_ENTRY) {
                fprintf(fd, "%-31s %-2s\n", labels[i].name, convertBase(labels[i].address));
            }
        }
        fclose(fd);
        free(fileplusextension);
    }
}

void create_object_file(char *filename) {
    char *fileplusextension;
    FILE *fd;
    Number i;
    fileplusextension = (char *) malloc(strlen(filename) + 4);
    if (!fileplusextension) {
        printf("memory allocation failed");
        exit(0);
    }
    strcpy(fileplusextension, filename);
    strcat(fileplusextension, ".ob");
    fd = fopen(fileplusextension, "w");
    fprintf(fd, "%6s %s %s\n", "", convertBase(cmdCount), convertBase(dataCount));
    for (i = 0; i < cmdCount + dataCount; ++i) {
        fprintf(fd, "%.7u %.7u %c\n", convertBase(i + ADDRESS_START), convertBase(code[i]), word_type[i]);
    }
    fclose(fd);
    free(fileplusextension);
}
