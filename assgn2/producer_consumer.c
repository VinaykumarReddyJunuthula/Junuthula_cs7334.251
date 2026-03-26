#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

enum {
    TAG_WORK = 1,
    TAG_ACK = 2,
    TAG_REQUEST = 3,
    TAG_ABORT = 4,
    TAG_NO_WORK = 5
};

static void do_fake_compute(int seed_value) {
    volatile double x = (double)(seed_value % 97 + 1);
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

    if (nproc < 4 || (nproc % 2) != 0) {
        if (rank == 0) {
            fprintf(stderr, "Please run with an even number of processes >= 4.\n");
        }
        MPI_Finalize();
        return 1;
    }

    int num_producers = (nproc - 1) / 2;
    int num_consumers = (nproc - 1) - num_producers;

    int is_broker = (rank == 0);
    int is_producer = (rank >= 1 && rank <= num_producers);
    int is_consumer = (rank >= num_producers + 1 && rank < nproc);

    srand((unsigned int)(time(NULL) + rank * 1337));

    long long consumed_count = 0;

    if (is_producer) {
        while (1) {
            int work = rand() % 100000 + 1;

            MPI_Request send_req;
            MPI_Isend(&work, 1, MPI_INT, 0, TAG_WORK, MPI_COMM_WORLD, &send_req);

            do_fake_compute(work);

            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            int broker_reply = 0;
            MPI_Status status;
            MPI_Recv(&broker_reply, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_ABORT) {
                break;
            }
        }
    } else if (is_consumer) {
        int previous_value = 1;

        while (1) {
            int request_value = 1;
            MPI_Request send_req;
            MPI_Isend(&request_value, 1, MPI_INT, 0, TAG_REQUEST, MPI_COMM_WORLD, &send_req);

            do_fake_compute(previous_value);

            MPI_Wait(&send_req, MPI_STATUS_IGNORE);

            int received_value = 0;
            MPI_Status status;
            MPI_Recv(&received_value, 1, MPI_INT, 0, MPI_ANY_TAG, MPI_COMM_WORLD, &status);

            if (status.MPI_TAG == TAG_WORK) {
                consumed_count++;
                previous_value = received_value;
            } else if (status.MPI_TAG == TAG_ABORT) {
                break;
            } else if (status.MPI_TAG == TAG_NO_WORK) {
            }
        }
    } else if (is_broker) {
        int buffer_size = 2 * num_producers;

        int *buffer = (int *)malloc(buffer_size * sizeof(int));
        int buf_head = 0, buf_tail = 0, buf_count = 0;

        int pending_capacity = num_producers + 4;
        int *pending_work = (int *)malloc(pending_capacity * sizeof(int));
        int *pending_src  = (int *)malloc(pending_capacity * sizeof(int));
        int pend_head = 0, pend_tail = 0, pend_count = 0;

        double start = MPI_Wtime();

        while ((MPI_Wtime() - start) < seconds) {
            int flag = 0;
            MPI_Status status;
            MPI_Iprobe(MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &flag, &status);

            if (!flag) {
                continue;
            }

            if (status.MPI_TAG == TAG_WORK) {
                int value = 0;
                MPI_Recv(&value, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (buf_count < buffer_size) {
                    buffer[buf_tail] = value;
                    buf_tail = (buf_tail + 1) % buffer_size;
                    buf_count++;

                    int ack = 1;
                    MPI_Send(&ack, 1, MPI_INT, status.MPI_SOURCE, TAG_ACK, MPI_COMM_WORLD);
                } else {
                    if (pend_count < pending_capacity) {
                        pending_work[pend_tail] = value;
                        pending_src[pend_tail] = status.MPI_SOURCE;
                        pend_tail = (pend_tail + 1) % pending_capacity;
                        pend_count++;
                    } else {
                        int abort_msg = -1;
                        MPI_Send(&abort_msg, 1, MPI_INT, status.MPI_SOURCE, TAG_ABORT, MPI_COMM_WORLD);
                    }
                }
            } else if (status.MPI_TAG == TAG_REQUEST) {
                int dummy = 0;
                MPI_Recv(&dummy, 1, MPI_INT, status.MPI_SOURCE, TAG_REQUEST,
                         MPI_COMM_WORLD, MPI_STATUS_IGNORE);

                if (buf_count > 0) {
                    int value = buffer[buf_head];
                    buf_head = (buf_head + 1) % buffer_size;
                    buf_count--;

                    MPI_Send(&value, 1, MPI_INT, status.MPI_SOURCE, TAG_WORK, MPI_COMM_WORLD);

                    if (pend_count > 0 && buf_count < buffer_size) {
                        int psrc = pending_src[pend_head];
                        int pval = pending_work[pend_head];
                        pend_head = (pend_head + 1) % pending_capacity;
                        pend_count--;

                        buffer[buf_tail] = pval;
                        buf_tail = (buf_tail + 1) % buffer_size;
                        buf_count++;

                        int ack = 1;
                        MPI_Send(&ack, 1, MPI_INT, psrc, TAG_ACK, MPI_COMM_WORLD);
                    }
                } else {
                    int no_work = 0;
                    MPI_Send(&no_work, 1, MPI_INT, status.MPI_SOURCE, TAG_NO_WORK, MPI_COMM_WORLD);
                }
            }
        }

        for (int p = 1; p < nproc; p++) {
            int abort_msg = -1;
            MPI_Send(&abort_msg, 1, MPI_INT, p, TAG_ABORT, MPI_COMM_WORLD);
        }

        free(buffer);
        free(pending_work);
        free(pending_src);
    }

    long long total_consumed = 0;
    MPI_Reduce(&consumed_count, &total_consumed, 1, MPI_LONG_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0) {
        printf("Total number of messages consumed: %lld\n", total_consumed);
    }

    MPI_Finalize();
    return 0;
}
