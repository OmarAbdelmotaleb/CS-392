/*******************************************************************************
 * Name        : pfind.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 03/16/21
 * Description : Finds files with a specified set of permissions. 
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <getopt.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>
#include "permfinder.h"

void usage_message(char *filename) {
	printf("Usage: %s -d <directory> -p <permissions string> [-h]\n", filename);
}

int main(int argc, char **argv) {

	if (argc == 1) {
		usage_message(argv[0]);
		return EXIT_FAILURE;
	}

	char directory[PATH_MAX+1];
	char permissions[PATH_MAX];
	int dflag = 0;
	int pflag = 0;
	int opt;
	opterr = 0; 
	
	while ((opt = getopt(argc, argv, ":d:p:h")) != -1) {
		switch (opt) {
			case 'd':
				strcpy(directory, optarg);
				dflag = 1;
				break;
			case 'h':
				usage_message(argv[0]);
				return EXIT_FAILURE;
			case 'p':
				strcpy(permissions, optarg);
				pflag = 1;
				break;
			case '?':
				printf("Error: Unknown option '-%c' received.\n", optopt);
				return EXIT_FAILURE;
		}
	} 

	if (dflag == 0) {
		puts("Error: Required argument -d <directory> not found.");
		return EXIT_FAILURE;
	}
	
	if (pflag == 0) {
		puts("Error: Required argument -p <permissions string> not found.");
		return EXIT_FAILURE;
	}
	
	struct stat sb;
	if (lstat(directory, &sb) < 0) {
		fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", directory, strerror(errno));
		return EXIT_FAILURE;
	}

	if(!S_ISDIR(sb.st_mode)) {
		printf("Error: '%s' is not a directory.\n", directory);
		return EXIT_FAILURE;
	}

	char path[PATH_MAX];
    if (realpath(directory, path) == NULL) {
        fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", argv[2], strerror(errno));
        return EXIT_FAILURE;
    } 

	perm_finder(path, permissions);	
	
	return 1;
 
}
