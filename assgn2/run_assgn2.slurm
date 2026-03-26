#!/bin/bash
#SBATCH -J assgn2
#SBATCH -A ASC23013
#SBATCH -N 2
#SBATCH -n 16
#SBATCH -p development
#SBATCH -t 02:00:00
#SBATCH -o assgn2.out
#SBATCH -e assgn2.err

cd $SLURM_SUBMIT_DIR

make clean
make all

echo "Producer-Consumer" > results.txt
for p in 4 8 12 16
do
  echo "$p processes:" >> results.txt
  for r in 1 2 3 4 5
  do
    ibrun -n $p ./producer_consumer 120 >> results.txt
  done
done

echo "" >> results.txt
echo "Work-Pool" >> results.txt
for p in 4 8 12 16
do
  echo "$p processes:" >> results.txt
  for r in 1 2 3 4 5
  do
    ibrun -n $p ./workpool 120 >> results.txt
  done
done
