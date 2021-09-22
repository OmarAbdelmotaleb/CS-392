pfind: permfinder.o pfind.o
	gcc permfinder.o pfind.o -o pfind
permfinder.o: permfinder.c permfinder.h
	gcc -Wall -Werror -pedantic-errors -c permfinder.c
pfind.o: pfind.c
	gcc -Wall -Werror -pedantic-errors -c pfind.c
clean:
	rm -f *.o pfind pfind.exe
