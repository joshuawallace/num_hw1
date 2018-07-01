CFLAGS = -Wall -std=c99 -g
OMPFLAGS = -fopenmp

.PHONY: clean

all:  main_omp main_mpi

main_omp: main_omp.c
	$(CC) $(CFLAGS) $(OMPFLAGS) $^ -o $@

main_mpi: main_mpi.c
	mpicc $(CFLAGS) -o $@ $^

clean:
	-rm -rf *.o main_omp main_mpi
