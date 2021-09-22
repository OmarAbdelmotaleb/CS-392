/*******************************************************************************
 * Name        : sort.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 03/02/21
 * Description : Uses quicksort to sort a file of either ints, doubles, or
 *               strings.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "quicksort.h"

#define MAX_STRLEN     64 // Not including '\0'
#define MAX_ELEMENTS 1024

typedef enum {
    STRING,
    INT,
    DOUBLE
} elem_t;

/**
 * Reads data from filename into an already allocated 2D array of chars.
 * Exits the entire program if the file cannot be opened.
 */
size_t read_data(char *filename, char **data) {
    // Open the file.
    FILE *fp = fopen(filename, "r");
    if (fp == NULL) {
        fprintf(stderr, "Error: Cannot open '%s'. %s.\n", filename,
                strerror(errno));
        free(data);
        exit(EXIT_FAILURE);
    }

    // Read in the data.
    size_t index = 0;
    char str[MAX_STRLEN + 2];
    char *eoln;
    while (fgets(str, MAX_STRLEN + 2, fp) != NULL) {
        eoln = strchr(str, '\n');
        if (eoln == NULL) {
            str[MAX_STRLEN] = '\0';
        } else {
            *eoln = '\0';
        }
        // Ignore blank lines.
        if (strlen(str) != 0) {
            data[index] = (char *)malloc((MAX_STRLEN + 1) * sizeof(char));
            strcpy(data[index++], str);
        }
    }

    // Close the file before returning from the function.
    fclose(fp);

    return index;
}

void usage_message(char *filename) {
	printf("Usage: %s [-i|-d] filename\n   -i: Specifies the file contains ints.\n   -d: Specifies the file contains double.\n   filename: The file to sort.\n   No flags defaults to sorting strings.\n", filename);
}

/**
 * Basic structure of sort.c:
 *
 * Parses args with getopt.
 * Opens input file for reading.
 * Allocates space in a char** for at least MAX_ELEMENTS strings to be stored,
 * where MAX_ELEMENTS is 1024.
 * Reads in the file
 * - For each line, allocates space in each index of the char** to store the
 *   line.
 * Closes the file, after reading in all the lines.
 * Calls quicksort based on type (int, double, string) supplied on the command
 * line.
 * Frees all data.
 * Ensures there are no memory leaks with valgrind. 
 */
int main(int argc, char **argv) {
    //Case 1: No input arguements
    if (argc == 1) {
    	usage_message(argv[0]);
    	return EXIT_FAILURE;
    }
    
    int iflag = 0;
    int dflag = 0;
    int flag_count = 0;
    int opt;
    int index;
    opterr = 0; //supresses getopt eror message
    
    
    while ((opt = getopt(argc, argv, "id")) != -1) {
    	switch (opt) {
    	  case 'i':
    	    iflag = 1;
    	    flag_count++;
    	    //printf("i flag set\n");
    	    break;
    	  case 'd':
    	    dflag = 1;
    	    flag_count++;
    	    //printf("d flag set\n");
    	    break;
    	  case '?':
    	  	//Case 2: Invalid flags & Case 7: Multiple flags w/ invalid flag
    	    printf("Error: Unknown option '%c' received.\n" , optopt);
    	    usage_message(argv[0]);
    	    return EXIT_FAILURE;
    	}
    }

    //Case 5: Multiple filenames
    int input_count = 0;
    for (index = optind; index < argc; index++) {
    	input_count++;
    	if (input_count > 1) {
  			printf("Error: Too many files specifid.\n");
  			return EXIT_FAILURE;	
    	}
    }
    
    //Case 4: No filename
    if (input_count == 0) {
    	printf("Error: No input file specified.\n");
    	return EXIT_FAILURE;
    }
    
    //Case 6: Multiple valid flags
    if (flag_count > 1) {
    	printf("Error: Too many flags specified.\n");
    	return EXIT_FAILURE;
    }
    
    
    char **data = (char**)malloc(MAX_ELEMENTS * sizeof(char*)); 
    
    
    size_t size = read_data(argv[optind], data);
    
    
   
    if (iflag == 1) {
    	int *int_data = malloc(size * sizeof(int));
    	
    	//copying data to an array of ints
    	for (int i = 0; i < size; i++) {
    		int_data[i]=atoi(data[i]);
    	}
    	
    	quicksort(int_data, size, sizeof(int), int_cmp);
    	
    	//printing sorted array
    	for (size_t i = 0; i < size; i++) {
    		printf("%d\n", int_data[i]); 
    	}
    	
    	//freeing memory
    	free(int_data);
    	for (int i = 0; i < size; i++) {
    		free(data[i]);
    	}
    	free(data);
    } else if (dflag == 1) {
    	double *dbl_data = malloc(size * sizeof(double));
    	
    	//copying data to an array of doubles
    	for (int i = 0; i < size; i++) {
    		dbl_data[i]=atof(data[i]);
    	}
    	
    	quicksort(dbl_data, size, sizeof(double), dbl_cmp);
    	
    	//printing sorted array
    	for (size_t i = 0; i < size; i++) {
    		printf("%f\n", dbl_data[i]); 
    	}
    	
    	//freeing memory
    	free(dbl_data);
    	for (int i = 0; i < size; i++) {
    		free(data[i]);
    	}
    	free(data);
    } else {
    	quicksort(data, size, sizeof(char*), str_cmp);
    	
    	//printing sorted array
    	for (size_t i = 0; i < size; i++) {
    		printf("%s\n", data[i]); 
    	}
    	
    	//freeing memory
    	for (int i = 0; i < size; i++) {
    		free(data[i]);
    	}
    	free(data);
    }
    
    
    return EXIT_SUCCESS;
}
