all: helpers bufio
helpers:
	gcc -c -std=c11 -Wall -Wextra -fpic helpers.c
	gcc -shared -o libhelpers.so helpers.o
bufio:
	gcc -c -std=c11 -Wall -Wextra -fpic bufio.c
	gcc -shared -o libbufio.so bufio.o
clean:
	rm -f *.o *.so
