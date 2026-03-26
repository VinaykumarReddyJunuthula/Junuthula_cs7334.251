Assignment 2 – MPI Programming
Vinaykumar Reddy Junuthula

----------------------------------------
Important Files
----------------------------------------

producer_consumer.c
- MPI implementation of producer-consumer model
- Rank 0 acts as broker

workpool.c
- MPI implementation of work-pool model
- All processes communicate directly

Makefile
- Compiles both programs using:
    make all

report.pdf
- Final report with results, chart, and observations

----------------------------------------
Compilation
----------------------------------------

make clean
make all

----------------------------------------
Execution
----------------------------------------

Producer-Consumer:
    ibrun -n <num_processes> ./producer_consumer 120

Work-Pool:
    ibrun -n <num_processes> ./workpool 120

Example:
    ibrun -n 4 ./producer_consumer 120
    ibrun -n 4 ./workpool 120

----------------------------------------
Notes
----------------------------------------

- Each program runs for 120 seconds
- Output format:
    Total number of messages consumed: Y
- Throughput = Y / 120
- Tested on TACC Lonestar6
