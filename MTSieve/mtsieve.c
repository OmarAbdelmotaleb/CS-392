/*******************************************************************************
 * Name        : mtsieve.c
 * Author      : Benjamin Singleton & Omar Abdelmotaleb
 * Date        : 04/23/21
 * Description : Standard sieve of Eratosthenes using multi-threading.
 * Pledge      : I pledge my honor that I have abided by the Stevens Honor System.
 ******************************************************************************/
#include <sys/sysinfo.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>
#include <limits.h>
#include <errno.h>
#include <math.h>
#include <pthread.h>

int total_count = 0;
pthread_mutex_t lock;

typedef struct arg_struct {
	int start;
	int end;
} thread_args;

int threeDigits(int num) {
	int count = 0;
	while(num != 0) {
		if(num % 10 == 3) {
			count ++;
		}
		num /= 10;
	}
	return count;
}

void *sieve(void *ptr) {
	thread_args targs = *(thread_args *)ptr;
	
	bool *low_primes = (bool *)malloc((sqrt(targs.end) + 1) * sizeof(bool));
	
	for (int i = 2; i < sqrt(targs.end); i++) {
		low_primes[i] = true;
	}
	
	for (int i = 2; i <= sqrt(targs.end); i++) {
		if (low_primes[i] == true) {
			for (int j = pow(i, 2); j <= sqrt(targs.end); j+=i) {
				low_primes[j] = false;
			}
		}
	}

	int length = (targs.end - targs.start + 1);
	bool *high_primes = (bool *)malloc(length * sizeof(bool));
	for (int i = 0; i < length; i++) {
		high_primes[i] = true;
	}
	
	for (int k = 2; k <= sqrt(targs.end); k++) {
		if (low_primes[k] == true) {
			int p = k;
			
			int i = ceil((double)targs.start/p) * p - targs.start;
			
			if (targs.start <= p) {
				i = i + p;
			}
			
			for (int j = i; j < length; j+=p) {
				high_primes[j] = false;
			}
		}
	}
	
	int retval;
	if ((retval = pthread_mutex_lock(&lock)) != 0) {
		fprintf(stderr, "Warning: Cannot lock mutex. %s.\n",
                strerror(retval));
	}
	
	for (int i = 0; i < length; i++) {
		if (high_primes[i] == true) {
			if (threeDigits(i+targs.start) >= 2) {
				total_count+=1;
			}
		}
	}
	
    if ((retval = pthread_mutex_unlock(&lock)) != 0) {
        fprintf(stderr, "Warning: Cannot unlock mutex. %s.\n",
                strerror(retval));
    }
    free(low_primes);
    free(high_primes);
	pthread_exit(NULL);
}


bool isInt(char *str) {
	for (int i = 0; i < strlen(str); i++) {
		if (isdigit(str[i]) == false) {
			return false;
		}	
	}
	return true;
}

int main(int argc, char **argv) {
	if (argc == 1) {
		printf("Usage: %s -s <starting value> -e <ending value> -t <num threads>\n",
				argv[0]);
		return EXIT_FAILURE; 
	}
	
	long long test;
	char *p;

	int starting_value;
	int ending_value;
	int num_threads = 0;
	int count;
	int remainder;
	int e_flag = 0;
	int s_flag = 0;
	int t_flag = 0;
	int n_procs= 0;
	int opt;
	
	opterr = 0; //supresses getopt error messages
	
	while ((opt = getopt(argc, argv, "e:s:t:")) != -1) {
		switch(opt) {
			case 'e':
				if (isInt(optarg) == false) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n",
							optarg, opt);
					return EXIT_FAILURE;
				}
				test = strtoll(optarg, &p, 10);
				if (test > INT_MAX) {
					fprintf(stderr, "Integer overflow for parameter '-%c'.\n",
							opt);
					return EXIT_FAILURE;
				}
				ending_value = atoi(optarg);
				e_flag = 1;
				break;
			case 's':
				if (isInt(optarg) == false) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n",
							optarg, opt);
					return EXIT_FAILURE;
				}
				test = strtoll(optarg, &p, 10);
				if (test > INT_MAX) {
					fprintf(stderr, "Integer overflow for parameter '-%c'.\n",
							opt);
					return EXIT_FAILURE;
				}
				starting_value = atoi(optarg);
				s_flag = 1;
				break;
			case 't':
				if (isInt(optarg) == false) {
					fprintf(stderr, "Error: Invalid input '%s' received for parameter '-%c'.\n",
							optarg, opt);
					return EXIT_FAILURE;
				}
				test = strtoll(optarg, &p, 10);
				if (test > INT_MAX) {
					fprintf(stderr, "Integer overflow for parameter '-%c'.\n",
							opt);
					return EXIT_FAILURE;
				}
				num_threads = atoi(optarg);
				t_flag = 1;
				break;
			case '?':
				if (optopt == 'e' || optopt == 's' || optopt == 't') {
					fprintf(stderr, "Error: Option -%c requires an argument.\n", optopt);
				} else if (isprint(optopt)) {
					fprintf(stderr, "Error: Unknown option '-%c'.\n", optopt);
				} else{
					fprintf(stderr, "Error: Unknown option character '\\x%x'.\n",
							optopt);
				}
				return EXIT_FAILURE;
		}
	}
	
	if (argv[optind] != NULL) {
		fprintf(stderr, "Error: Non-option argument '%s' supplied.\n",
				argv[optind]);
		return EXIT_FAILURE;
	}
	
	if (s_flag == 0) {
		fprintf(stderr, "Error: Required argument <starting value> is missing.\n");
		return EXIT_FAILURE;
	}
	
	if (starting_value < 2) {
		fprintf(stderr, "Error: Starting value must be >= 2.\n");
		return EXIT_FAILURE;
	}
	
	if (e_flag == 0) {
		fprintf(stderr, "Error: Required argument <ending value> is missing.\n");
		return EXIT_FAILURE;
	}
	
	if (ending_value < 2) {
		fprintf(stderr, "Error: Ending value must be >= 2.\n");
		return EXIT_FAILURE;
	}
	
	if (ending_value < starting_value) {
		fprintf(stderr, "Error: Ending value must be >= starting value.\n");
		return EXIT_FAILURE;
	}
	
	if (t_flag == 0) {
		fprintf(stderr, "Error: Required argument <num threads> is missing.\n");
		return EXIT_FAILURE;
	}
	
	if (num_threads < 1) {
		fprintf(stderr, "Error: Number of threads cannot be less than 1.\n");
		return EXIT_FAILURE;
	}
	
	n_procs = get_nprocs();
	if (num_threads > (2 * n_procs)) {
		fprintf(stderr, "Error: Number of threads cannot exceed twice the number of processors(%d).\n", n_procs);
		return EXIT_FAILURE;
	}
	
	count = ending_value - starting_value + 1;
	if (num_threads > count) {
		num_threads = count;
	} 
	count = (ending_value - starting_value + 1)/num_threads;
	remainder = (ending_value - starting_value + 1)%num_threads;
	
	
	int *seg_sizes = (int *)malloc(num_threads * sizeof(int));
	for (int i = 0; i < num_threads; i++) {
		seg_sizes[i] = count;
	
	}

	int loop_count = 0;
	while (remainder > 0) {
		seg_sizes[loop_count]+=1;
		remainder--;
		loop_count++;
		if (loop_count == num_threads) {
			loop_count = 0;
		}
	}
	
	printf("Finding all prime numbers between %d and %d.\n", 
			starting_value, ending_value);
	if (num_threads == 1) {
		printf("1 segment:\n");
	} else {
		printf("%d segments:\n", num_threads);
	}
	
	
	thread_args *targs = (thread_args *)malloc(num_threads * sizeof(thread_args));
	int accum = starting_value;
	for (int i = 0; i < num_threads; i++) {
		targs[i].start = accum;
		targs[i].end = (accum + seg_sizes[i] -1);
		accum = accum + seg_sizes[i];
	}
	
	
	for (int i = 0; i < num_threads; i++) {
		printf("   [%d, %d]\n", targs[i].start, targs[i].end);
	}
	
	
	pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
	int retval;
	for (int i = 0; i < num_threads; i++) {
		if ((retval = pthread_create(&threads[i], NULL, 
			 sieve, (void*)(&targs[i]))) != 0) {
			 fprintf(stderr, "Error: Cannot create thread %d. %s.\n",
			 		 i + 1, strerror(retval));
		}
	}
	
	for (int i = 0; i < num_threads; i++) {
		if (pthread_join(threads[i], NULL) != 0) {
			fprintf(stderr, "Warning: Thread %d did not join properly.\n",
                    i + 1);
		}
	}
	
    if ((retval = pthread_mutex_destroy(&lock)) != 0) {
        fprintf(stderr, "Error: Cannot destroy mutex. %s.\n", strerror(retval));
        free(seg_sizes);
		free(targs);
		free(threads);
        return EXIT_FAILURE;
    }	
	


	printf("Total primes between %d and %d with two or more '3' digits: %d\n",
			starting_value, ending_value, total_count);
	free(seg_sizes);
	free(targs);
	free(threads);
	return EXIT_SUCCESS;
}
