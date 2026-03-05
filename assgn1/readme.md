# Assignment 1 – OpenMP Performance Study

Author: Vinaykumar Reddy Junuthula  
Course: CS 7334.251 – HPC@Scale  
Texas State University

This repository contains the code, results, and report for Assignment 1.  
The assignment analyzes the performance of a parallel π computation using OpenMP on the Lonestar6 cluster.


## Important Files

compute_pi.c  
OpenMP implementation of the π computation program.

batch_submission.sh  
SLURM batch script used to run experiments on Lonestar6.

report.pdf  
Final assignment report explaining the experiments and results.

report.tex  
LaTeX source file used to generate the report.


## Results Folder

results/

hwloc_lonestar6.txt  
Hardware topology information collected using the hwloc tool.

lscpu_lonestar6.txt  
CPU architecture details of the compute node.

results_affinity.csv  
Execution times for different OpenMP thread affinity policies.

results_scaling.csv  
Execution times for strong scaling experiments using different numbers of threads.

results_schedule.csv  
Execution times comparing static and dynamic scheduling policies.


## Performance Plots

plot_affinity.png  
Plot showing performance of different affinity policies.

plot_scaling.png  
Plot showing execution time vs number of threads.

plot_schedule.png  
Plot comparing static and dynamic scheduling performance.


## Other Files

compute_pi  
Compiled executable.

*.out and *.err files  
Output logs generated from SLURM job runs.

report.aux and report.log  
Temporary files generated during LaTeX compilation.
