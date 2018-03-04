#ifndef __FIRST_PASS_H__
#define __FIRST_PASS_H__

#include "worker.h"

/* Firstpass implements the first pass over the code as described in the mmn instructions */
 
/* Analyze the next line in the file including all the first pass responcebilities as described in the mmn. */
/* Looks for errors, and return TRUE if the we need to continue compiling. */
BOOL firstpass_analyze_line(FILE *fd);

/* Return the number of words the command will take (or 0 if error) and checking for errors along the way*/
Number number_of_words(char* line);

#endif /* __FIRST_PASS_H__ */
