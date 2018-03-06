#ifndef __USEFUL_H__
#define __USEFUL_H__

#include "structs.h"
#include "bits.h"

/* useful functions for the entire program*/

/* Prints an error massage according to the error array */
void print_err(Number err);

/* extracts a number and puts it in outparam. Return 0 if success, or -1 if failure */
Number extract_num(char* stringform, Number* outparam);

/* Puts the first argument of input (source) into dest,dest needs to be allocated somewhere else, i'm assuming we have
   up to destlen characters in it.
   The return value is 'source' advanced beyond its first argument (and the whitespace characters after it)
*/
char *separate_arg(char *source, char *dest, Number destlen);

/* Return 1 if label was previously discovered, 2 if it's in external labels list, 0 if nowhere */
label* get_label(char *label);

/* Return 0 if it's a valid label */
Number verify_label(char* label);

/* Checks if it is a register */
BOOL is_register(char* arg);

/* Returns the number of the register "arg" if it's a register or -1 if not */
Number extract_register(char* arg);

/* converts to a 32-base */
char *convertBase(Number number);

#endif /* __UTILS_H__ */
