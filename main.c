/*
 *  Maman 14
 *  main.c
 *  Created on: Feb, 2018
 *  Author: Shir Cohen
 *  ID: 200948347
 *  This module implements the main functions of the program
 */

#include <stdio.h>
#include "worker.h"


int main(int argc, char *argv[]) {
    int i = 1;
    int wasSuccessful = 0;
/* Checking that we received input files */
    if (argc < 2) {
        printf("%s Please enter files in this format: {filename} [filename...]\n", argv[0]);
        return 0;
    }
/* Goes through all the input files */
    while (i < argc) {
        printf("Compiling %s...\n", argv[i]);
        if (!assemble(argv[i])) {
            printf("Compiled successfully.\n");
        } else {
            printf("Error during compilation of %s.\n", argv[i]);
            wasSuccessful = 1;
        }
        ++i;
    }
    return wasSuccessful;
}