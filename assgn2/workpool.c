#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    TAG_WORK = 1,
    TAG_STOP = 2
};

static void do_fake_compute(int seed_value) {
    volatile double x = (double)(seed_value % 113 + 1);
    for (int i = 0; i < 5000; i++) {
        x = x * 1.0000001 + 0.000001;
        x = x / 1.00000001 + 0.0000001;
    }
    (void)x;
}

int main(int argc, char *argv[]) {
    MPI_Init(&argc, &argv);

    int rank, nproc;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nproc);

    if (argc < 2) {
        if (rank == 0) {
            fprintf(stderr, "Usage: %s <seconds>\n", argv[0]);
        }
        MPI_Finalize();
        return 1;
    }

    int seconds = atoi(argv[1]);
    if (seconds <= 0) {
        if (rank == 0) {
            fprintf(stderr, "Seconds must be positive.\n");
        }
        MPI_Finalize();
        return 1;
    }

    srand((unsigned int)(time(NULL) + rank * 977));

    long long consumed_count = 0;
    double start = MPI_Wtime();

    while ((MPI_Wtime() - start) < seconds) {
        int work = rand() % 100000 + 1;
        int dest = rand() % nproc;

        MPI_Send(&work, 1, MPI_INT, dest, TAG_WORK, MPI_COMM_WORLD);

        int flag = 0;
        MPI_Status status;

        while (1) {
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
            if (!flag) {
                break;
            }

            if (status.MPI_TAG == TAG_WORK) {
                int incoming = 0;
                MPI_Recv(&incoming, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                do_fake_compute(incoming);
                consumed_count++;
            }
        }

        do_fake_compute(work);
    }

    MPI_Barrier(MPI_COMM_WORLD);

    int draining = 1;
    while (draining) {
        int flag = 0;
        MPI_Status status;
        MPI_Iprobe(MPI_ANY_SOURCE, TAG_WORK, MPI_COMM_WORLD, &flag, &status);

        if (flag) {
            int incoming = 0;
            MPI_Recv(&incoming, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
            do_fake_compute(incoming);
            consumed_count++;
        } else {
            draining = 0;
        }
    }

    long long total_consumed = 0;
    MPI_Reduce(&consumed_count, &total_consumed, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total number of messages consumed: %lld\n", total_consumed);
    }

    MPI_Finalize();
    return 0;
}
