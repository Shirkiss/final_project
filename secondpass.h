#ifndef __SECOND_PASS_H__
#define __SECOND_PASS_H__

#include "worker.h"

/* Secondpass implements the second pass over the code as described in the mmn instructions */

/* Analyze the next line in the file including all the second pass responcebilities as described in the mmn. */
/* Looks for errors, and return TRUE if the we need to continue compiling. */
BOOL secondpass_analyze_line(FILE* fd);

/* Fill in the code with the correct bytes, fill up the external label references array */
Number handle_command_line_secondpass(char* line);

#endif /* __SECOND_PASS_H__ */
