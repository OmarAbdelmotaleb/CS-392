sort: quicksort.o sort.o
	gcc quicksort.o sort.o -o sort
quicksort.o: quicksort.c quicksort.h
	gcc -Wall -Werror -pedantic-errors -c quicksort.c
sort.o: sort.c
	gcc -Wall -Werror -pedantic-errors -c sort.c
clean:
	rm -f *.o sort sort.exe
