/*
 *  Maman 14
 *  secondpass.c
 *  Created on: july, 2013
 *  Author: Michal Blumenthal
 *  ID: 302940432
 *  This module implements the second pass over the code as described in the mmn instructions
 */

#include <stdlib.h>
#include <string.h>
#include "useful.h"
#include "secondpass.h"

extern char *strdup(const char *__s);

extern Number code[CODE_SIZE], data[CODE_SIZE], cmdCount, dataCount, line_number, ext_count;
extern char word_type[CODE_SIZE];
extern label *ext_address;
extern const command commands[];

/* Analyze the next line in the file including all the second pass responcebilities as described in the mmn. */
/* Looks for errors, and return TRUE if the we need to continue compiling. */
/* Now we have all the labels, fill in the memory map, place the commands where
   they should, and then place the data after it */
BOOL secondpass_analyze_line(FILE *fd) {
    char current_line[MAX_BUFFER];
    char label[MAX_LABEL_LENGTH];
    char *next_arg;
    Number temp, i;
    BOOL label_exist = FALSE;
    if (fgets(current_line, MAX_BUFFER, fd)) {    /* put \0 at the end of the line */
        temp = strlen(current_line);
        current_line[temp - 1] = '\0';

        if (is_useless_line(current_line)) {
            line_number++;
            return TRUE;
        }
        next_arg = separate_arg(current_line, label, MAX_LABEL_LENGTH);
        temp = strlen(label);
        label_exist = (label[temp - 1] == ':' ? TRUE : FALSE);
        /* If it's a compiler directive */
        if (is_compiler_directive((label_exist) ? next_arg : current_line)) {
            line_number++;
            return TRUE;
        }
        if ((temp = handle_command_line_secondpass((label_exist) ? next_arg : current_line)) != 0) {
            print_err(temp);
        }
        line_number++;
        return TRUE;
    } else {
        ++cmdCount;
        /* Add the data segment right after the code segment :) */
        for (i = 0; i < dataCount; ++i) {
            code[i + cmdCount] = data[i];
            word_type[i + cmdCount] = ' ';
        }
        /* done with second pass */
        return FALSE;
    }
}

/* Fill in the code with the correct bytes, fill up the external label references array */
Number handle_command_line_secondpass(char *line) {
    char *command;
    char *first_arg;
    char *second_arg;
    char *type;
    char *first_comb;
    char *sec_comb;
    char *dbl;
    char temp_line[MAX_BUFFER];
    Number i, temp, bitsFirstComb = 0, bitsSecComb = 0, bothComb = 0, currCommand;
    label *current_label, *current_label_index;
    char *struct_name;
    char *struct_arg_num;
    int bitsType, bitsDbl;
    if (!line || *line == '\0')
        return ERROR_COMMAND_NULL;
    strcpy(temp_line, line);
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
        return ERROR_TOO_MANY_PARAMETERS;
    }
    if (commands[i].num_of_arguments >= 1 && first_arg == NULL) {
        return ERROR_MISSING_PARAMETERS;
    }
    if (commands[i].num_of_arguments < 2 && second_arg != NULL) {
        return ERROR_TOO_MANY_PARAMETERS;
    }
    if (commands[i].num_of_arguments >= 2 && second_arg == NULL) {
        return ERROR_MISSING_PARAMETERS;
    }
    if (commands[i].num_of_arguments < 1 && first_arg != NULL) {
        return ERROR_TOO_MANY_PARAMETERS;
    }
    ++cmdCount;
    currCommand = cmdCount;
    code[currCommand] = (commands[i].opcode << OPCODE_SHIFT) & OPCODE_MASK;
    code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
    if (commands[i].num_of_arguments == 0) {
        return 0;
    }
    if (commands[i].num_of_arguments == 1) {
        if (first_arg[0] == '#') {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_INSTANT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            if (extract_num(first_arg + 1, &temp) != -1) {
                /* Jump past the opcode word into the operand word */
                code[++cmdCount] = temp; /* Fill in the operand and jump cmdCount past it */
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
                return 0;
            } else {
                return (ERROR_BAD_NUMBER);
            }
        } else
            /* The first operand is a register or a label */
        if (is_register(first_arg)) {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_REGISTER_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            code[currCommand] |= ADDRESS_METHOD_REGISTER_DIRECT << DESTINATION_ADDRESSING_METHOD_SHIFT;
            return 0;
        } else
            /* it's not a register but a label. */
        if ((current_label = get_label(first_arg)) != NULL) {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            code[currCommand] |= ADDRESS_METHOD_DIRECT << DESTINATION_ADDRESSING_METHOD_SHIFT;
            code[++cmdCount] = current_label->address;
            code[currCommand] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;

            if (current_label->type == LABEL_TYPE_EXTERNAL) {
                /* Save the address of the external label for the ext file */
                ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                ext_address[ext_count].name = strdup(current_label->name);
                ext_address[ext_count].address = ADDRESS_START + cmdCount;
                ext_count++;
                code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
            }
            return 0;
        } else if ((struct_name = strtok(first_arg, ".")) != NULL) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                return ERROR_INVALID_STRUCT_FORMAT;
            }

            /* Get the struct label */
            if ((current_label = get_label(struct_name)) == NULL) {
                return ERROR_LABEL_DOESNT_EXIST;
            } else {
                code[++cmdCount] = current_label->address;
                code[currCommand] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;

                if (current_label->type == LABEL_TYPE_EXTERNAL) {
                    /* Save the address of the external label for the ext file */
                    ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                    ext_address[ext_count].name = strdup(current_label->name);
                    ext_address[ext_count].address = ADDRESS_START + cmdCount;
                    ext_count++;
                    code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
                }
            }
            if (extract_num(struct_arg_num, &temp) != -1) {
                code[currCommand] |= ADDRESS_METHOD_ENTRY_ACCESS << DESTINATION_ADDRESSING_METHOD_SHIFT;
                code[++cmdCount] = temp;
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            } else {
                return ERROR_INVALID_STRUCT_FORMAT;
            }
            return 0;
        } else {
            return (ERROR_LABEL_DOESNT_EXIST);
        }
    } else {
        /* Process the argument which may be a label (which may be defined or undefined),
           a register, a number (with a sign and an addressing method indicator) */
        if (first_arg[0] == '#') {
            if (commands[i].legal_source_addressing_methods[ADDRESS_METHOD_INSTANT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            /* Read a number now. That is the first operand, the addressing method is 0,
               an extra word will keep this number */
            if (extract_num(first_arg + 1, &temp) != -1) {
                code[++cmdCount] = temp;
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            }
        } else if (is_register(first_arg)) {
            if (commands[i].legal_source_addressing_methods[ADDRESS_METHOD_REGISTER_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            temp = extract_register(first_arg);
            code[currCommand] |= ADDRESS_METHOD_REGISTER_DIRECT << SOURCE_ADDRESSING_METHOD_SHIFT;
        } else if ((current_label = get_label(first_arg)) != NULL) {
            if (commands[i].legal_source_addressing_methods[ADDRESS_METHOD_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            code[currCommand] |= ADDRESS_METHOD_DIRECT << SOURCE_ADDRESSING_METHOD_SHIFT;
            code[++cmdCount] = ((current_label->address) - currCommand);
            code[currCommand] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            if (current_label->type == LABEL_TYPE_EXTERNAL) {
                /* Save the address of the external label for the ext file */
                ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                ext_address[ext_count].name = strdup(current_label->name);
                ext_address[ext_count].address = ADDRESS_START + cmdCount;
                ext_count++;
                code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
            }
        } else if ((struct_name = strtok(first_arg, ".")) != NULL) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                return ERROR_INVALID_STRUCT_FORMAT;
            }
            /* Get the struct label */
            if ((current_label = get_label(struct_name)) == NULL) {
                return ERROR_LABEL_DOESNT_EXIST;
            } else {
                code[++cmdCount] = (current_label->address << ADDRESS_SHIFT) & ADDRESS_MASK;
                code[cmdCount] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;
                code[currCommand] |= ADDRESS_METHOD_ENTRY_ACCESS << SOURCE_ADDRESSING_METHOD_SHIFT;

                if (current_label->type == LABEL_TYPE_EXTERNAL) {
                    /* Save the address of the external label for the ext file */
                    ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                    ext_address[ext_count].name = strdup(current_label->name);
                    ext_address[ext_count].address = ADDRESS_START + cmdCount;
                    ext_count++;
                    code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
                }
            }
            if (extract_num(struct_arg_num, &temp) != -1) {
                code[++cmdCount] = temp;
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            } else {
                return ERROR_INVALID_STRUCT_FORMAT;
            }
        } else {
            return ERROR_LABEL_DOESNT_EXIST;
        }
        /* Process the 2nd argument, it may be a label, register, or a number */
        if (second_arg[0] == '#') {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_INSTANT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            /* Read a number */
            if (extract_num(second_arg + 1, &temp) != -1) {
                code[++cmdCount] = temp;
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
                return 0;
            }
        } else
            /* The first operand is a register or a label. */
        if (is_register(second_arg)) {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_REGISTER_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            code[currCommand] |= (ADDRESS_METHOD_REGISTER_DIRECT << DESTINATION_ADDRESSING_METHOD_SHIFT);
            return 0;
        } else
            /* It's a label. */
        if ((current_label = get_label(second_arg)) != NULL) {
            if (commands[i].legal_dest_addressing_methods[ADDRESS_METHOD_DIRECT] == FALSE) {
                return (ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE);
            }
            code[currCommand] |= ADDRESS_METHOD_DIRECT << DESTINATION_ADDRESSING_METHOD_SHIFT;
            code[++cmdCount] = current_label->address;
            code[currCommand] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            if (current_label->type == LABEL_TYPE_EXTERNAL) {
                /* Save the address of the external label for the ext file */
                ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                ext_address[ext_count].name = strdup(current_label->name);
                ext_address[ext_count].address = ADDRESS_START + cmdCount;
                ext_count++;
                code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
            }
            return 0;
        } else if ((struct_name = strtok(second_arg, ".")) != NULL) {
            if ((struct_arg_num = strtok(NULL, " ")) == NULL) {
                return ERROR_INVALID_STRUCT_FORMAT;
            }

            /* Get the struct label */
            if ((current_label = get_label(struct_name)) == NULL) {
                return ERROR_LABEL_DOESNT_EXIST;
            } else {
                code[++cmdCount] = current_label->address;
                code[currCommand] |= (RELOCATABLE_TYPE << TYPE_SHIFT) & TYPE_MASK;
                if (current_label->type == LABEL_TYPE_EXTERNAL) {
                    /* Save the address of the external label for the ext file */
                    ext_address = realloc(ext_address, sizeof(label) * (ext_count + 1));
                    ext_address[ext_count].name = strdup(current_label->name);
                    ext_address[ext_count].address = ADDRESS_START + cmdCount;
                    ext_count++;
                    code[currCommand] |= (EXTERNAL_TYPE << TYPE_SHIFT) & TYPE_MASK;
                }
            }
            if (extract_num(struct_arg_num, &temp) != -1) {
                code[currCommand] |= ADDRESS_METHOD_ENTRY_ACCESS << DESTINATION_ADDRESSING_METHOD_SHIFT;
                code[++cmdCount] = temp;
                code[currCommand] |= (ABSOLUTE_TYPE << TYPE_SHIFT) & TYPE_MASK;
            } else {
                return ERROR_INVALID_STRUCT_FORMAT;
            }
            return 0;
        } else {
            return ERROR_LABEL_DOESNT_EXIST;
        }
    }
    return 1;
}