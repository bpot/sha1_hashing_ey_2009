#LIBS=-lprofiler -lcrypto
LIBS=-lcrypto
CFLAGS=-O3 
main: next_gen.o my_sha.o rounds.o
	gcc -O3  -o next_gen $(LIBS) next_gen.o my_sha.o rounds.o
