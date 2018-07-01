CFLAGS = -Wall -std=c99 -g
OMPFLAGS = -fopenmp

.PHONY: clean

all: main_serial main_omp main_mpi

%.o: %.c
	$(CC) -c $(CFLAGS) $(OMPFLAGS) -o $@ $^

main_serial: main_serial.o
	$(CC) $(CFLAGS) -o $@ $^ 

main_omp: main_omp.o
	$(CC) $(CFLAGS) $(OMPFLAGS) $^ -o $@

main_mpi: main_mpi.c
	mpicc $(CFLAGS) -o $@ $^

clean:
	-rm -rf *.o main_serial main_omp main_mpi
