#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    TAG_WORK = 1,
    TAG_ACK = 2
};

static void do_fake_compute(int seed_value) {
    volatile double x = (double)(seed_value % 113 + 1);
    for (int i = 0; i < 5000; i++) {
        x = x * 1.0000001 + 0.000001;
        x = x / 1.00000001 + 0.0000001;
    }
    (void)x;
}

static void drain_incoming(long long *consumed_count) {
    int flag = 0;
    MPI_Status status;

    while (1) {
        MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);
        if (!flag) {
            break;
        }

        if (status.MPI_TAG == TAG_WORK) {
            int value = 0;
            MPI_Recv(&value, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);

            do_fake_compute(value);

            int ack = 1;
            MPI_Send(&ack, 1, MPI_INT, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
            (*consumed_count)++;
        } else if (status.MPI_TAG == TAG_ACK) {
            int ack = 0;
            MPI_Recv(&ack, 1, MPI_INT, status.MPI_SOURCE, TAG_ACK,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }
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

        MPI_Request send_req;
        MPI_Isend(&work, 1, MPI_INT, dest, TAG_WORK, MPI_COMM_WORLD, &send_req);

        int ack_received = 0;

        while (!ack_received) {
            int flag = 0;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (!flag) {
                do_fake_compute(work);
                continue;
            }

            if (status.MPI_TAG == TAG_WORK) {
                int incoming = 0;
                MPI_Recv(&incoming, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                do_fake_compute(incoming);

                int ack = 1;
                MPI_Send(&ack, 1, MPI_INT, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
                consumed_count++;
            } else if (status.MPI_TAG == TAG_ACK) {
                int ack = 0;
                MPI_Recv(&ack, 1, MPI_INT, status.MPI_SOURCE, TAG_ACK,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                ack_received = 1;
            }
        }

        MPI_Wait(&send_req, MPI_STATUS_IGNORE);
    }

    for (int round = 0; round < 3; round++) {
        drain_incoming(&consumed_count);
        MPI_Barrier(MPI_COMM_WORLD);
    }

    long long total_consumed = 0;
    MPI_Reduce(&consumed_count, &total_consumed, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total number of messages consumed: %lld\n", total_consumed);
    }

    MPI_Finalize();
    return 0;
}
