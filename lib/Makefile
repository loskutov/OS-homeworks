all:
	gcc -c -std=c11 -Wall -fpic helpers.c
	gcc -shared -o libhelpers.so helpers.o
clean:
	rm -f *.o *.so
