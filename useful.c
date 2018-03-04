/*
 *  Maman 14
 *  firstpass.c
 *  Created on: Feb, 2018
 *  Author: Shir Cohen
 *  ID: 200948347
 *  This module implements useful functions for the entire program
 */

#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "useful.h"
#include "worker.h"

extern Number line_number;
BOOL was_error;

typedef struct {
    const char *errtoprint;
    const Number errnum;
} error_struct;

const error_struct compiler_errors[] =
        {
                {"Error in line %d: ",                                               ERROR_START_ERROR},
                {"Labels cannot start with non abc characters.",                     ERROR_LABEL_START_NONABC},
                {"Labels cannot contain non abc characters.",                        ERROR_LABEL_CONTAINS_NONABC},
                {"Label length too long.",                                           ERROR_LABEL_TOO_LONG},
                {"Label already defined elsewhere.",                                 ERROR_LABEL_ALREADY_DEFINED},
                {"Label is a keyword.",                                              ERROR_LABEL_IS_KEYWORD},
                {"Unknown label error.",                                             ERROR_LABEL_UNKNOWN_ERROR},
                {"Missing label name before \":\".",                                 ERROR_LABEL_MISSING_NAME},
                {"Label does not exist.",                                            ERROR_LABEL_DOESNT_EXIST},
                {"Label declared as an entry point but not defined in the program.", ERROR_LABEL_DECLARED_AS_ENTRY_BUT_NOT_DEFINED},
                {"Too many parameters",                                              ERROR_TOO_MANY_PARAMETERS},
                {"Missing parameters.",                                              ERROR_MISSING_PARAMETERS},
                {"Missing a string.",                                                ERROR_MISSING_STRING},
                {"Bad number format.",                                               ERROR_BAD_NUMBER},
                {"Null command.",                                                    ERROR_COMMAND_NULL},
                {"Command doesn't exist.",                                           ERROR_COMMAND_NOT_FOUND},
                {"The operand addressing method is ilegal for this command.",        ERROR_COMMAND_OPERAND_ADDRESSING_METHOD_UNACCEPTABLE},
                {"Invalid struct format.",                                           ERROR_INVALID_STRUCT_FORMAT},
                {"Missing combination.",                                             ERROR_MISSING_COMB},
                {"Invalid combination.",                                             ERROR_INVALID_COMB},
                {0,                                                                  0}
        };

extern label *labels; /* stores labels */
extern Number num_of_labels;
extern const command commands[]; /* stores commands */

/* Prints an error massage accordinf to the error array */
void print_err(Number err) {
    Number i;
    printf(compiler_errors[ERROR_START_ERROR].errtoprint, line_number + 1);
    for (i = 1; i < ERROR_MAX_ERROR; ++i)
        if (compiler_errors[i].errnum == err)
            printf("%s", compiler_errors[i].errtoprint);
    printf("\n");
    was_error = TRUE;
}

char reVal(int num) {
    if (num >= 0 && num <= 9)
        return (char) (num + '0');
    else
        return (char) (num - 10 + 'A');
}

/* Utility function to reverse a string */
void strev(char *str) {
    int len = strlen(str);
    int i;
    for (i = 0; i < len / 2; i++) {
        char temp = str[i];
        str[i] = str[len - i - 1];
        str[len - i - 1] = temp;
    }
}

/* extracts a number and puts it in outparam. Return 0 if success, or -1 if failure */
Number extract_num(char *stringform, Number *outparam) {
    Number minus = 0;
    Number result = 0;
    Number i = 0;
    if (!stringform || *stringform == '\0')
        return -1;
    if (!outparam)
        return -1;
    /* Handles +'s and -'s */
    if (stringform[0] == '+') {
        ++stringform;
    } else {
        if (stringform[0] == '-') {
            ++stringform;
            minus = 1;
        }
    }
    for (i = 0;; ++i) {
        if (stringform[i] == '\0') {
            break;
        }

        if (!isdigit(stringform[i])) {
            return -1;
        }

        result *= 10;
        result += stringform[i] - '0';
    }
    if (minus) {
        result = -result;
    }
    *outparam = result;
    return 0;
}

/* Save the first argument of the source into dest (can be up to destlen).
 * Return the given source after the first argument (with the whitespace characters after it)
 */
char *separate_arg(char *source, char *dest, Number destlen) {
    --destlen;
    /* moves after the spaces */
    while (isspace(*source))
        source++;
    while ((*source != '\0') && (destlen != 0)) {
        /* If this is the end of the first word -> break */
        if (isspace(*source)) {
            source++;
            break;
        }
        *dest++ = *source++;
        destlen--;
    }
    *dest = '\0';
    while (isspace(*source))
        source++;
    return source;
}


/* Return 1 if label was previously discovered, 2 if it's in external labels list, 0 if nowhere */
label *get_label(char *label) {
    Number i = 0;
    if (!label || label[0] == '\0')
        return 0;
    while (i < num_of_labels) {
        if (strcmp(labels[i].name, label) == 0) {
            return &labels[i];
        }
        i++;
    }
    return NULL;
}

/* Return 0 if it's a valid label */
Number verify_label(char *label) {
    Number i;
    if (!label || label[0] == '\0')
        return ERROR_LABEL_MISSING_NAME;
    /* checks that the label starts with a letter */
    if (!isalpha(label[0]))
        return ERROR_LABEL_START_NONABC;
    for (i = 1; label[i] != '\0'; ++i) {
        /* Checks if the passed character is alphanumeric */
        if (!isalnum(label[i])) {
            if (label[i] == '\n') {
                label[i] = '\0';
                break;
            } else {
                return ERROR_LABEL_CONTAINS_NONABC;
            }
        }
    }
    i = 0;
    /* checks if the name of the label is name of command */
    while (commands[i].name != NULL) {
        if (strcmp(commands[i].name, label) == 0) {
            return ERROR_LABEL_IS_KEYWORD;
        }
        i++;
    }
    /* checks if the name of the lable is a register */
    if (label[0] == 'r' && ((label[1] - '0') < NUMBER_OF_REGISTERS) && ((label[1] - '0') >= 0) && label[2] == '\0')
        return ERROR_LABEL_IS_KEYWORD;
    /*all good!*/
    return 0;
}

/* Checks if it is a register */
BOOL is_register(char *arg) {
    if (!arg || arg[0] == '\0')
        return FALSE;
    if (arg[0] == 'r') {
        if (arg[1] - '0' >= 0 && arg[1] - '0' < NUMBER_OF_REGISTERS) {
            if (arg[2] == '\0')
                return TRUE;
        }
    }
    return FALSE;
}

/* Returns the number of the register "arg" if it's a register or -1 if not */
Number extract_register(char *arg) {
    Number i;
    if (is_register(arg)) {
        extract_num(arg + 1, &i);
        return i;
    }
    return -1;
}


/*recives 0 or 1 that represents the side of the 10 bits to read and returns the combination*/
Number combination(Number firstComb, Number secComb) {
    if (firstComb && secComb) {
        return BOTH_RIGHT;
    }
    if (firstComb && !secComb) {
        return SRC_RIGHT_DEST_LEFT;
    }
    if (!firstComb && secComb) {
        return SRC_LEFT_DEST_RIGHT;
    }
    if (!firstComb && !secComb) {
        return BOTH_LEFT;
    }
    return 0;
}

/*calculating number of bits used */
int bitforint() {
    unsigned int x = 1;
    int c = 0;
    while (x != 0) {
        x = x << 1;
        c = c + 1;
    }
    return c;
}


/* converts to a dec base */
unsigned int toEight(Number number) {
    unsigned int tmp = 0;
    Number number_copy = number;
    unsigned int tmpNeg = 0;
    Number c = 1;
    unsigned int mask = 1;
    int i;
    mask = mask << (bitforint() - 1);
    /* checks if the number is negetive */
    if ((mask & number) != 0) {
        mask = 1;
        for (i = 0; i < (bitforint() - 1); i++) {
            number_copy = (number_copy ^ mask);
            mask = mask << 1;
        }
        number_copy++;
        tmpNeg = number_copy;
    } else
        tmpNeg = number;
    while ((tmpNeg != 0) && (c <= 1000000000)) {
        tmp += c * (tmpNeg % 8);
        c *= 10;
        tmpNeg /= 8;
    }
    return tmp;
}

/* converts to a 32-base */
char *convertBase(Number number) {
    char *res;
    int index = 0;  /* Initialize index of result */

    res = malloc(sizeof(char) * 3);

    /* Convert input number to 32 base by repeatedly
     dividing it by base and taking remainder */
    while (number > 0) {
        res[index++] = reVal(number % 32);
        number /= 32;
    }
    res[index] = '\0';

    /* Reverse the result */
    strev(res);

    return res;

}
