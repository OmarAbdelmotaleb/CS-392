/*******************************************************************************
 * Name        : spfind.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 03/31/21
 * Description : Finds files with a specified set of permissions and sorts the output.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

bool starts_with(const char *str, const char *prefix) {
    return strncmp(prefix, str, strlen(prefix)) == 0;
}

int main(int argc, char *argv[]) {
	
	int pfind_to_sort[2], sort_to_parent[2];
	if (pipe(pfind_to_sort) < 0) {
		fprintf(stderr, "Error: Cannot create pipe pfind_to_sort. %s.\n",
				strerror(errno));
		return EXIT_FAILURE;
	}
	
	if (pipe(sort_to_parent) < 0) {
		fprintf(stderr, "Error: Cannot create pipe sort_to_parent. %s.\n",
				strerror(errno));
		return EXIT_FAILURE;
	}
	
	
	pid_t pid[2];
	if ((pid[0] = fork()) == 0) {

		close(pfind_to_sort[0]);
		dup2(pfind_to_sort[1], STDOUT_FILENO);
		
		close(sort_to_parent[0]);
		close(sort_to_parent[1]);
		
		if (execvp("./pfind", argv) < 0) {
			fprintf(stderr, "Error: pfind failed. %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

	}
	
	if ((pid[1] = fork()) == 0) {
		
		close(pfind_to_sort[1]);
		dup2(pfind_to_sort[0], STDIN_FILENO);
		close(sort_to_parent[0]);
		dup2(sort_to_parent[1], STDOUT_FILENO);

		if (execlp("sort", "sort", argv[1], NULL) < 0) {
			fprintf(stderr, "Error: sort failed. %s.\n", strerror(errno));
			exit(EXIT_FAILURE);
		}

	}
	
	close(sort_to_parent[1]);
	dup2(sort_to_parent[0], STDIN_FILENO);
	
	close(pfind_to_sort[0]);
	close(pfind_to_sort[1]);
	
	char buffer[4096];
	int counter = 0;
	int usage = 0;
	while (1) {

		ssize_t count = read(STDIN_FILENO, buffer, sizeof(buffer));
		if (count == -1) {
			if (errno == EINTR) {
				continue;
			} else {
				perror("read()");
				exit(EXIT_FAILURE);
			}
		} else if (count == 0) {
			break;
		} else {
			write(STDOUT_FILENO, buffer, count);
			if (starts_with(buffer, "Usage: ")) {
				usage = 1;
			}
			for (int i = 0; i < count; i++) {
				if(buffer[i] == '\n') {
					counter++;
				}
			}
		}

	}

	close(sort_to_parent[0]);

	int status;
	for(int i = 0; i < 2; i++) {
		pid_t w = waitpid(pid[i], &status, 0);
		if (w == -1) {
			// waitpid failed.
			perror("waitpid()");
			exit(EXIT_FAILURE);
		}
		if (WIFEXITED(status) && WEXITSTATUS(status)) {
			return EXIT_FAILURE;
		}
	}

	if(!usage) {
		printf("Total matches: %d\n", counter);
	}
	
	return EXIT_SUCCESS;
}
