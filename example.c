// SPDX-License-Identifier: MIT
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// Github:  @San7o

#define MICRO_BENCH_IMPLEMENTATION
#include "micro-bench.h"

int fib(int x)
{
  if (x == 0) return 0;
  if (x == 1) return 1;
  return fib(x-1) + fib(x-2);
}

int main(void)
{
  MicroBench mb;

  micro_bench_clear(&mb);

  printf("Calculating fibonacci numbers...\n");
  for (volatile int i = 0; i < 10; ++i)
  {
    micro_bench_start(&mb);

    // Do something...
    fib(35);
    
    micro_bench_stop(&mb);
  }

  double mean_real = micro_bench_get_mean_real(&mb);
  double mean_cpu = micro_bench_get_mean_cpu(&mb);
  printf("\nMean real time is: %f seconds\n", mean_real);
  printf("\nMean cpu time is: %f seconds\n", mean_cpu);

  // Print min, max, mean, variance, sum...
  micro_bench_report(&mb);
  
  // or use a specific reporter
  printf("\nCSV exporter:\n");
  micro_bench_report_with(&mb, micro_bench_default_reporter_csv);
  
  return 0;
}
