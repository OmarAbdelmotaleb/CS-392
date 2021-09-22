/*******************************************************************************
 * Name        : minishell.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 04/14/21
 * Description : Basic shell program in C with commands, signal handling, and colors.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pwd.h>

#define BRIGHTBLUE "\x1b[34;1m"
#define DEFAULT    "\x1b[0m"

volatile sig_atomic_t got_signal = 0;

void catch_sig(int sig) {
	got_signal = sig;
    write(1, "\n", 1);
}

int main() {
    
    char** m_argv = malloc(PATH_MAX * sizeof(char*));
    if (m_argv == NULL) {
        fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    }

	char* cwd = (char *)malloc(PATH_MAX);
    if (cwd == NULL) {
        fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
        free(m_argv);
        return EXIT_FAILURE;
    }
    
	if (getcwd(cwd, PATH_MAX) == NULL) {
		fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
		return EXIT_FAILURE;
	}

    struct sigaction action;
    memset(&action, 0, sizeof(struct sigaction));
    action.sa_handler = catch_sig;
    if (sigaction(SIGINT, &action, NULL) == -1) {
        fprintf(stderr, "Error: Cannot register signal handler. %s.\n", strerror(errno));
        return EXIT_FAILURE;
    } 

	while(1) {     

        free(m_argv);
        m_argv = malloc(PATH_MAX * sizeof(char*));
        if (m_argv == NULL) {
            fprintf(stderr, "Error: malloc() failed. %s.\n", strerror(errno));
            free(cwd);
            return EXIT_FAILURE;
        }
	    printf("[%s%s%s]$ ", BRIGHTBLUE, cwd, DEFAULT);
        fflush(stdout);
		char input[PATH_MAX];
        int read_val = read(STDIN_FILENO, input, PATH_MAX);
        // Put return value into variable
        // Ctrl + D stops program
		if(read_val < 0){
            if(errno == EINTR) {
                continue;
            }
            else {
                fprintf(stderr, "Error: Failed to read from stdin. %s.\n", strerror(errno));
                return EXIT_FAILURE;
            }
        } else if (read_val == 0) {
            write(1, "\n", 1);
            break;
        }
        
		input[strcspn(input, "\n")] = 0;
        
        int i = 0;
        int m_argc = 0;
        char *p = strtok(input, " ");
        while(p != NULL) {
            m_argv[i++] = p;
            p = strtok(NULL, " ");
            m_argc++;
        }
        m_argv[m_argc] = NULL;        
        

        struct passwd *pwd = getpwuid(getuid());
        if (pwd == NULL) {
        	fprintf(stderr, "Error: Cannot get passwd entry. %s.\n", strerror(errno));
            free(m_argv);
            free(cwd);
            exit(EXIT_FAILURE);
        }

        if (m_argc > 0) {
        
            if (strcmp(m_argv[0], "cd") == 0) {

                if (m_argc == 1) {
                    // set path to ~
                    chdir(pwd->pw_dir);
                    if (getcwd(cwd, PATH_MAX) == NULL) {
                        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
                        free(m_argv);
                        free(cwd);
                        exit(EXIT_FAILURE);
                    }	
                }
                else if (m_argc == 2) {

                    if(strcmp(m_argv[1], "~") == 0) {
                        m_argv[1] = pwd->pw_dir;
                    } else if (m_argv[1][0] == '~') {
                        char *k = m_argv[1];
                        k++;
                        char *b = pwd->pw_dir;
                        strcat(b, k);
                        strcpy(m_argv[1], k);
                    }
                    
                    if (chdir(m_argv[1]) == -1) {
                    	fprintf(stderr, "Error: Cannot change directory to '%s'. %s.\n",
                    			m_argv[1], strerror(errno));
                    }
                    if (getcwd(cwd, PATH_MAX) == NULL) {
                        fprintf(stderr, "Error: Cannot get current working directory. %s.\n", strerror(errno));
                        free(m_argv);
                        free(cwd);
                        exit(EXIT_FAILURE);
                    }	
                    
                }
                else {
                    fprintf(stderr, "Error: Too many arguments to cd.\n");
                }

            }
            else if (strcmp(m_argv[0], "exit") == 0){
                break;
            }
            else {

                pid_t pid;
                   
                if ((pid = fork()) < 0) {
                    fprintf(stderr, "Error: fork() failed. %s.\n", strerror(errno));
                    continue;
                } else if (pid == 0) {
                    if (execvp(m_argv[0], m_argv) == -1) {
                        fprintf(stderr, "Error: exec() failed. %s.\n", strerror(errno));
                        free(m_argv);
                        free(cwd);
                        exit(EXIT_FAILURE);
                    }
                }

                pid_t child_pid;
                while ((child_pid = wait(NULL)) == -1 && errno == EINTR) {
                    continue;
                }

                if(child_pid < 0) {
                    fprintf(stderr, "Error: wait() failed. %s.\n", strerror(errno));
                    free(m_argv);
                    free(cwd);
                    exit(EXIT_FAILURE);
                }
            }
        }
	}

    free(m_argv);
    free(cwd);

    return EXIT_SUCCESS;
   
}
