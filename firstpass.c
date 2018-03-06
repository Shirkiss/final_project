/*
 *  Maman 14
 *  firstpass.c
 *  Created on: Feb, 2018
 *  Author: Shir Cohen
 *  ID: 200948347
 *  This module implements the first pass over the code as described in the mmn instructions
 */

#include "firstpass.h"
#include "useful.h"
#include "structs.h"
#include <string.h>
#include <stdlib.h>

extern char *strdup(const char *__s);

extern Number cmdCount, dataCount, line_number, num_of_labels;
extern label *labels; /* Hold labels*/
extern const command commands[]; /* Hold commands */

/* Analyze the next line in the file including all the first pass responcebilities as described in the mmn. */
/* looks for errors, and return TRUE if the we need to continue compiling. */
BOOL firstpass_analyze_line(FILE *fd) {
    char current_line[MAX_BUFFER], first_arg[MAX_BUFFER];
    char *next_arg;
    label *current_label;
    Number errno;
    int i, j;
    Number temp, curr_loc, data_len;
    BOOL valid_label = FALSE;
    curr_loc = ADDRESS_START;
    /* Read a line */
    if (fgets(current_line, MAX_BUFFER, fd)) {
        /* End it with \0 instead of a \n */
        temp = strlen(current_line);
        current_line[temp - 1] = '\0';
        /* Check if the line is useless or not (have only white spaces or comma)*/
        if (is_useless_line(current_line)) {
            line_number++;
            return TRUE;
        }
        /* next_arg holds either a command line (minus a "label:" in the beginning)
           or the command's arguments (if there was no label) */
        next_arg = separate_arg(current_line, first_arg, MAX_LABEL_LENGTH + 1);
        /* First arg holds the "label:" if there was one, or the command if there wasn't one */
        if (first_arg[strlen(first_arg) - 1] == ':') {
            /* Remove the ':' from the label */
            first_arg[strlen(first_arg) - 1] = '\0';
            if ((errno = verify_label(first_arg)) == 0) {
                current_label = get_label(first_arg);
                if (current_label != NULL && current_label->address == -1 && current_label->type == LABEL_TYPE_ENTRY) {
                    /* Valid label - kept in first_arg */
                    valid_label = TRUE;
                } else if (current_label == NULL) {
                    valid_label = TRUE;
                } else {
                    print_err(ERROR_LABEL_ALREADY_DEFINED);
                    valid_label = FALSE;
                }
            } else {
                next_arg = separate_arg(next_arg, first_arg, MAX_LABEL_LENGTH);
                print_err(errno);
            }
        }
        /* We have the existing, non-existing or invalid label.
           If valid_label == FALSE, first_arg contains the compiler directive or command we need to analyze,
           otherwise it contains a label and next_arg contains the next argument(s) in the line */
        if ((errno = is_compiler_directive((valid_label ? next_arg : first_arg)))) {
            if (errno == DIRECTIVE_DATA || errno == DIRECTIVE_STRING || errno == DIRECTIVE_STRUCT) {
                if (valid_label) {
                    /* 'next_arg' will hold all the parameters and without any optional starting label or the directive */
                    next_arg = separate_arg(next_arg, current_line, MAX_BUFFER - 1);
                }
                if (is_useless_line(next_arg)) {
                    print_err(ERROR_MISSING_PARAMETERS);
                    return TRUE;
                }
                /* If you handled the compiler directive successfully and you need to store a label */
                if (((data_len = (get_compiler_directive_args_len(next_arg, errno))) != -1) && valid_label) {
                    if (current_label == NULL) {
                        /* Insert the new label to DC since it will be pointing at data */
                        labels = realloc(labels, sizeof(label) * (num_of_labels + 1));
                        if (!labels) {
                            printf("memory allocation failed");
                            exit(0);
                        }
                        /* Push the new label to the labels list*/
                        labels[num_of_labels].name = strdup(first_arg);
                        labels[num_of_labels].address = dataCount - 1;
                        labels[num_of_labels].type = LABEL_TYPE_REGULAR;
                        labels[num_of_labels].is_data_label = TRUE;
                        labels[num_of_labels].length = data_len;
                        ++num_of_labels;
                    } else {
                        /*If existing label -> update data*/
                        current_label->address = dataCount - 1;
                        current_label->is_data_label = TRUE;
                        current_label->length = data_len;
                    }
                }
            } else if (errno == DIRECTIVE_EXTERN) {
                /* If it's an extern directive, store the following label as an external label */
                next_arg = separate_arg((valid_label ? next_arg : current_line), first_arg, MAX_LABEL_LENGTH);
                current_label = get_label(next_arg);
                if (((errno = verify_label(next_arg)) == 0) && (current_label == NULL)) {
                    /* Add it as an external label */
                    labels = realloc(labels, sizeof(label) * (num_of_labels + 1));
                    if (!labels) {
                        printf("memory allocation failed");
                        exit(0);
                    }
                    labels[num_of_labels].name = strdup(next_arg);
                    labels[num_of_labels].address = 0;
                    labels[num_of_labels].type = LABEL_TYPE_EXTERNAL;
                    labels[num_of_labels].length = 0;
                    ++num_of_labels;
                } else {
                    if (current_label != NULL) {
                        print_err(ERROR_LABEL_ALREADY_DEFINED);
                    } else {
                        print_err(errno);
                    }
                }

                next_arg = separate_arg(next_arg, first_arg, MAX_LABEL_LENGTH);
                if (*next_arg != '\0') {
                    print_err(ERROR_TOO_MANY_PARAMETERS);
                }
            } else if (errno == DIRECTIVE_ENTRY) {
                next_arg = separate_arg((valid_label ? next_arg : current_line), first_arg, MAX_LABEL_LENGTH);
                if ((errno = verify_label(next_arg)) == 0) {
                    current_label = get_label(next_arg);
                    /* Either we encountered a new label in this declaration */
                    if (current_label == NULL) {
                        /* Add it as an entry point */
                        labels = realloc(labels, sizeof(label) * (num_of_labels + 1));
                        if (!labels) {
                            printf("memory allocation failed");
                            exit(0);
                        }
                        labels[num_of_labels].name = strdup(next_arg);
                        labels[num_of_labels].address = -1; /* We'll fill this in later */
                        labels[num_of_labels].type = LABEL_TYPE_ENTRY;
                        labels[num_of_labels].is_data_label = FALSE;
                        ++num_of_labels;
                    } else {
                        /* Or an old one that is just now being declared as an entry point */
                        if (current_label->type == LABEL_TYPE_REGULAR) {
                            current_label->type = LABEL_TYPE_ENTRY;
                        } else {
                            print_err(ERROR_LABEL_ALREADY_DEFINED);
                        }
                    }
                } else {
                    print_err(errno);
                }
                next_arg = separate_arg(next_arg, first_arg, MAX_LABEL_LENGTH);
                if (*next_arg != '\0') {
                    print_err(ERROR_TOO_MANY_PARAMETERS);
                }
            }
        }
            /* errno is not a directive */
        else {
            curr_loc = ADDRESS_START + cmdCount;
            if (valid_label) {
                if (current_label != NULL) {
                    if (current_label->address == -1 && current_label->type == LABEL_TYPE_ENTRY) {
                        current_label->is_data_label = FALSE;
                        current_label->address = curr_loc;
                    }
                } else {
                    /* Insert the previously discovered label to DC since it will be pointing at data */
                    labels = realloc(labels, sizeof(label) * (num_of_labels + 1));
                    if (!labels) {
                        printf("memory allocation failed");
                        exit(0);
                    }
                    labels[num_of_labels].name = strdup(first_arg);
                    labels[num_of_labels].address = curr_loc;
                    labels[num_of_labels].type = LABEL_TYPE_REGULAR;
                    labels[num_of_labels].is_data_label = FALSE;
                    ++num_of_labels;
                }
            }
            cmdCount += number_of_words((valid_label) ? next_arg : current_line);
        }
    } else {
        /*sort by code position*/
        for (i = 0; i < num_of_labels ; ++i) {
            for (j = i + 1; j < num_of_labels; ++j) {
                if (labels[i].address > labels[j].address) {
                    label temp_label = labels[i];
                    labels[i] = labels[j];
                    labels[j] = temp_label;
                }
            }
        }
        curr_loc = ADDRESS_START + cmdCount;
        /* We need to offset all the data labels */
        for (line_number = 0; line_number < num_of_labels; ++line_number) {
            if (labels[line_number].is_data_label) {
                labels[line_number].address = curr_loc;
                curr_loc += labels[line_number].length;
            }
        }
        cmdCount = 0;
        line_number = 0;
        return FALSE; /* Done compiling the file for the first pass*/
    }
    ++line_number;
    return TRUE;
}

/* Return the number of words the command will take (or 0 if error) and checking for errors along the way*/
Number number_of_words(char *line) {
    char *command;
    char *first_arg;
    char *second_arg;
    char *struct_arg_num;
    char temp_line[MAX_BUFFER];
    Number i, temp;
    if (!line || *line == '\0')
        return 0;
    strcpy(temp_line, line);
    /*get the command from the line*/
    command = strtok(temp_line, "/\t \n,");
    if (*command == '\0')
        return ERROR_COMMAND_NULL;
    /* checking it is a valid command */
    for (i = 0; commands[i].name != NULL; ++i) {
        if (!strcmp(command, commands[i].name)) {
            break;
        }
    }
    if (commands[i].name == NULL) {
        return ERROR_COMMAND_NOT_FOUND;
    }

    /*analyzing arguments of command */
    first_arg = strtok(NULL, " \t,\n");
    second_arg = strtok(NULL, " \t,\n");
    if (strtok(NULL, " \t,\n") != NULL) {
        print_err(ERROR_TOO_MANY_PARAMETERS);
        return 0;
    }
    if (commands[i].num_of_arguments >= 1 && first_arg == NULL) {
        print_err(ERROR_MISSING_PARAMETERS);
        return 0;
    }
    if (commands[i].num_of_arguments < 2 && second_arg != NULL) {
        print_err(ERROR_TOO_MANY_PARAMETERS);
        return 0;
    }
    if (commands[i].num_of_arguments >= 2 && second_arg == NULL) {
        print_err(ERROR_MISSING_PARAMETERS);
        return 0;
    }
    if (commands[i].num_of_arguments < 1 && first_arg != NULL) {
        print_err(ERROR_TOO_MANY_PARAMETERS);
        return 0;
    }
    /* Process the arguments of the command */
    if (commands[i].num_of_arguments == 0) {
        return 1;
    }
    if (commands[i].num_of_arguments == 1) {
        /* Only one argument and it's a literal number. This command will take 2 words */
        if (first_arg[0] == '#') {
            return 2;
        } else
            /* Neither of the previous methods were used. This means the first operand is either
               a register or a label. */
        if (is_register(first_arg)) {
            /* There is no extra word. This means only 1 word is taken here */
            return 1;
        } else
            /* Check if it is a struct. */
        if ((strchr(first_arg, '.') != NULL) && ((strtok(first_arg, ".")) != NULL)) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                print_err(ERROR_INVALID_STRUCT_FORMAT);
                return 0;
                /*make sure the the struct arg is a number*/
            } else if (extract_num(struct_arg_num, &temp) != -1) {
                return 3;
            }
        } else {
            /* Finally, this means it's a register or a label. There's 1 extra word for it */
            return 2;
        }
    } else {
        i = 1;
        /* This command has TWO operands */
        /* The first operand is a literal number. This command will take 2 words at least */
        if (first_arg[0] == '#') {
            i++;
            /*Check if it is struct*/
        } else if ((strchr(first_arg, '.') != NULL) && ((strtok(first_arg, ".")) != NULL)) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                print_err(ERROR_INVALID_STRUCT_FORMAT);
                return 0;
                /*make sure the the struct arg is a number*/
            } else if (extract_num(struct_arg_num, &temp) != -1) {
                i += 2;
            }
        } else if (!((is_register(first_arg)) & (is_register(second_arg)))) {
            /* Finally, this means it's is a label or a register (but not 2 registers). There's 1 extra word for it */
            i++;
        }
        /* Perform the same check for the second argument */
        /* The second operand is a literal number, so this will take an additional word */
        if (second_arg[0] == '#') {
            i++;
            /*Check if it is struct*/
        } else if ((strchr(second_arg, '.') != NULL) && ((strtok(second_arg, ".")) != NULL)) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                print_err(ERROR_INVALID_STRUCT_FORMAT);
                return 0;
                /*make sure the the struct arg is a number*/
            } else if (extract_num(struct_arg_num, &temp) != -1) {
                i += 2;
            }
        } else {
            /* Finally, this means it's is a register or a label. There's 1 extra word for it */
            i++;
        }
    }
    return i;
}
