#!/bin/bash
#SBATCH -A ASC23013
#SBATCH -J assgn1_pi
#SBATCH -o assgn1_pi.%j.out
#SBATCH -e assgn1_pi.%j.err
#SBATCH -N 1
#SBATCH -n 1
#SBATCH -c 128
#SBATCH -p normal
#SBATCH -t 00:20:00

module purge
module load gcc
module load hwloc

mkdir -p results

# compile
gcc -O3 -fopenmp -o compute_pi compute_pi.c

# capture system info (Task 1)
lstopo-no-graphics --of txt > results/hwloc_lonestar6.txt
lscpu > results/lscpu_lonestar6.txt

N=10000000
export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK

echo "places,bind,threads,time_s" > results/results_affinity.csv
for places in core socket; do
  for bind in close spread; do
    export OMP_PLACES=$places
    export OMP_PROC_BIND=$bind
    t=$(./compute_pi $N | awk '/ComputeTime/ {print $3}')
    echo "$places,$bind,$OMP_NUM_THREADS,$t" >> results/results_affinity.csv
  done
done

export OMP_PLACES=core
export OMP_PROC_BIND=close

echo "threads,time_s" > results/results_scaling.csv
for t in 1 2 4 8 16 32 64 128; do   # adjust max if needed
  if [ $t -le $SLURM_CPUS_PER_TASK ]; then
    export OMP_NUM_THREADS=$t
    time_s=$(./compute_pi $N | awk '/ComputeTime/ {print $3}')
    echo "$t,$time_s" >> results/results_scaling.csv
  fi
done

export OMP_NUM_THREADS=$SLURM_CPUS_PER_TASK
echo "policy,chunk,time_s" > results/results_schedule.csv
for policy in static dynamic; do
  for chunk in 10 100 1000; do
    export OMP_SCHEDULE="$policy,$chunk"
    time_s=$(./compute_pi $N | awk '/ComputeTime/ {print $3}')
    echo "$policy,$chunk,$time_s" >> results/results_schedule.csv
  done
done
