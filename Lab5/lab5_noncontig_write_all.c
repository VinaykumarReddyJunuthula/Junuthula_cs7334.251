#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define FILESIZE 1024
#define INTS_PER_BLK 1

int main(int argc, char **argv)
{
    int *buf, *readbuf;
    int rank, nprocs, nints, bufsize;
    int i, num_blocks;
    MPI_File fh;
    MPI_Datatype filetype;
    char filename[] = "lab5_output.dat";

    MPI_Init(&argc, &argv);

    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &nprocs);

    bufsize = FILESIZE / nprocs;
    nints = bufsize / sizeof(int);

    if (nints <= 0) {
        if (rank == 0) {
            printf("Error: too many processes for FILESIZE.\n");
        }
        MPI_Finalize();
        return 1;
    }

    if (nints % INTS_PER_BLK != 0) {
        if (rank == 0) {
            printf("Error: nints must be divisible by INTS_PER_BLK.\n");
        }
        MPI_Finalize();
        return 1;
    }

    buf = (int *) malloc(nints * sizeof(int));
    readbuf = (int *) malloc(nints * sizeof(int));

    if (!buf || !readbuf) {
        printf("Rank %d: malloc failed\n", rank);
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    for (i = 0; i < nints; i++) {
        buf[i] = rank;
        readbuf[i] = -1;
    }

    num_blocks = nints / INTS_PER_BLK;

    MPI_Type_vector(num_blocks, INTS_PER_BLK, nprocs * INTS_PER_BLK,
                    MPI_INT, &filetype);
    MPI_Type_commit(&filetype);

    MPI_File_open(MPI_COMM_WORLD, filename,
                  MPI_MODE_CREATE | MPI_MODE_WRONLY,
                  MPI_INFO_NULL, &fh);

    MPI_File_set_view(fh,
                      rank * INTS_PER_BLK * sizeof(int),
                      MPI_INT,
                      filetype,
                      "native",
                      MPI_INFO_NULL);

    MPI_File_write_all(fh, buf, nints, MPI_INT, MPI_STATUS_IGNORE);

    MPI_File_close(&fh);

    MPI_File_open(MPI_COMM_WORLD, filename,
                  MPI_MODE_RDONLY,
                  MPI_INFO_NULL, &fh);

    MPI_File_set_view(fh,
                      rank * INTS_PER_BLK * sizeof(int),
                      MPI_INT,
                      filetype,
                      "native",
                      MPI_INFO_NULL);

    MPI_File_read_all(fh, readbuf, nints, MPI_INT, MPI_STATUS_IGNORE);

    MPI_File_close(&fh);

    printf("Rank %d wrote/read %d ints. First 5 values read: ", rank, nints);
    for (i = 0; i < nints && i < 5; i++) {
        printf("%d ", readbuf[i]);
    }
    printf("\n");

    MPI_Type_free(&filetype);
    free(buf);
    free(readbuf);

    MPI_Finalize();
    return 0;
}
