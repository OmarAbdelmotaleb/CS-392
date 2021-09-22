/*******************************************************************************
 * Name        : permfinder.h
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

int perm_checker(char *perm) {
	int permlen = strlen(perm);
	if (permlen != 9) {
		return 0;
	}
	
	for (int i = 0; i < permlen; i++) {
		if ((i%3 == 0) && (perm[i] != 'r' && perm[i] != '-')) {
			return 0;
		} else if ((i%3 == 1) && (perm[i] != 'w' && perm[i] != '-')) {
			return 0;
		} else if ((i%3 == 2) && (perm[i] != 'x' && perm[i] != '-')) {
			return 0;
		} 
	}
	
	return 1;
}	
int perms[] = {S_IRUSR, S_IWUSR, S_IXUSR,
               S_IRGRP, S_IWGRP, S_IXGRP,
               S_IROTH, S_IWOTH, S_IXOTH};

/*
 * Return 0 when permissions are different,
 * return 1 when permissions are the same.
*/
int perm_cmp(struct stat *statbuf, char *permissions) {

	char* perm_str = (char*)malloc(10 * sizeof(char*));
	perm_str[9] = '\0';
	int permission_valid;
	
	for (int i = 0; i < 9; i += 3) {
		permission_valid = statbuf->st_mode & perms[i];
		if (permission_valid) {
			perm_str[i] = 'r';
		} else {
			perm_str[i] = '-';
		}
		
		permission_valid = statbuf->st_mode & perms[i+1];
		if (permission_valid) {
			perm_str[i+1] = 'w';
		} else {
			perm_str[i+1] = '-';
		} 
		
		permission_valid = statbuf->st_mode & perms[i+2];
		if (permission_valid) {
			perm_str[i+2] = 'x';
		} else {
			perm_str[i+2] = '-';
		}	
	}
	
	if (strcmp(perm_str, permissions) == 0) {
		free(perm_str);
		return 1;
	} else {
		free(perm_str);
		return 0;
	}	

}

int perm_finder(char *path, char *permissions) {

	DIR *dir;
	struct dirent *dent;
	struct stat sb;
	
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "Error: Cannot open directory '%s'. %s.\n", path, strerror(errno));
        return EXIT_FAILURE;
    }
    
	errno = 0;
	while ((dent = readdir(dir)) != NULL) {

		char buffer[PATH_MAX+1];
		strcpy(buffer, path);
		strcat(buffer, "/");
		strcat(buffer, dent->d_name);
		
		if (lstat(buffer, &sb) < 0) {
			fprintf(stderr, "Error: Cannot stat '%s'. %s.\n", dent->d_name, strerror(errno));
			return EXIT_FAILURE;
		}
		if (!perm_checker(permissions)) {
			printf("Error: Permissions string '%s' is invalid.\n", permissions);
			return EXIT_FAILURE;
		}
		/* Check if the directory entry is a directory itself */
		
		if (S_ISDIR(sb.st_mode)) {
			if(strcmp(dent->d_name, ".") != 0 && strcmp(dent->d_name, "..") != 0) {
				if (perm_cmp(&sb, permissions) == 1) {
					printf("%s\n", buffer);
				}
				perm_finder(buffer, permissions);
			} 	
		} 
		
		else {
			if (perm_cmp(&sb, permissions) == 1) {
				printf("%s\n", buffer);
			}
		}
		
		errno = 0;
		
	}    

	closedir(dir);
    
    return EXIT_SUCCESS;

}


