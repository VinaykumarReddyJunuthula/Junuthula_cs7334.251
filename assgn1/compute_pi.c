#include <stdio.h>
#include <stdlib.h>
#include <omp.h>

int main(int argc, char *argv[]) {
  if (argc < 2) {
    printf("Usage: %s num_steps\n", argv[0]);
    return 1;
  }

  long num_steps = atol(argv[1]);
  double step = 1.0 / (double)num_steps;
  double sum = 0.0;

  double t0 = omp_get_wtime();

  #pragma omp parallel
  {
    double local_sum = 0.0;
    #pragma omp for
    for (long i = 0; i < num_steps; i++) {
      double x = (i + 0.5) * step;
      local_sum += 4.0 / (1.0 + x * x);
    }
    #pragma omp atomic
    sum += local_sum;
  }

  double t1 = omp_get_wtime();

  double pi = step * sum;

  printf("Pi = %.12f\n", pi);
  printf("ComputeTime(s) = %.6f\n", (t1 - t0));
  return 0;
}
