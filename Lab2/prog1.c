#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank = -1, size = 0;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 2) {
        if (rank == 0) {
            printf("Error: prog1 requires at least 2 MPI processes.\n");
        }
        MPI_Finalize();
        return 0;
    }

    int msg = -1;
    const int TAG = 0;

    if (rank == 0) {
        msg = rank;
        MPI_Send(&msg, 1, MPI_INT, 1, TAG, MPI_COMM_WORLD);
        printf("Rank %d sent value %d to Rank 1\n", rank, msg);
    } else if (rank == 1) {
        MPI_Recv(&msg, 1, MPI_INT, 0, TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank %d received value %d from Rank 0\n", rank, msg);
    }

    MPI_Finalize();
    return 0;
}

