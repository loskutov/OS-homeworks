all:
	gcc -c -Wall -fpic helpers.c
	gcc -shared -o libhelpers.so helpers.o
clean:
	rm -f *.o *.so
