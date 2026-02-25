#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank = -1, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Error: prog2 (ring) requires at least 2 MPI processes.\n");
        }
        MPI_Finalize();
        return 0;
    }

    int prev = (rank - 1 + size) % size;
    int next = (rank + 1) % size;
    const int TAG = 99;

    int value = -1;

    if (rank == 0) {
        srand((unsigned)time(NULL));
        value = rand() % 1000;

        printf("Rank 0 generated value %d\n", value);

        MPI_Send(&value, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);
        printf("Rank 0 sent value %d to Rank %d\n", value, next);

        MPI_Recv(&value, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 0 received value %d back from Rank %d (ring complete)\n", value, prev);
    } else {
        MPI_Recv(&value, 1, MPI_INT, prev, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d received value %d from Rank %d\n", rank, value, prev);

        MPI_Send(&value, 1, MPI_INT, next, TAG, MPI_COMM_WORLD);
        printf("Rank %d sent value %d to Rank %d\n", rank, value, next);
    }

    MPI_Finalize();
    return 0;
}

