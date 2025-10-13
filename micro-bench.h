//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// micro-bench.h
// =============
//
// Header-only benchmarking library in C99.
//
// Author:  Giovanni Santini
// Mail:    giovanni.santini@proton.me
// License: MIT
//
//
// Example code
// ------------
//
// ```
// MicroBench mb = {0};  // define a micro benchmark
//
// for (int i = 0; i < 10; ++i) // Repeat a few times
// {
//    micro_bench_start(&mb); // start benchmark
//    fib(35);                // Do something...
//    micro_bench_stop(&mb);  // end benchmark
// }
//
// micro_bench_report(&mb);   // the default reporter prints to stdout
// ```
//
// Example output
// --------------
// 
//
//         |---------------------------------------|
//         |         Micro benchmark report        |
//         |---------------------------------------|
//         |   ////   |     real     |     CPU     |
//         |---------------------------------------|
//         |   min    |  0.0814034   |  0.0810770  |
//         |   max    |  0.0958261   |  0.0954550  |
//         |   sum    |  0.8521938   |  0.8493760  |
//         |   mean   |  0.0852194   |  0.0849376  |
//         |   var    |  0.0000229   |  0.0000226  |
//         |---------------------------------------|
//         |   iterations   |           10         |
//         |---------------------------------------|
//
//
// Usage
// -----
//
// Do this:
//
//   #define MICRO_BENCH_IMPLEMENTATION
//
// before you include this file in *one* C or C++ file to create the
// implementation.
//
// i.e. it should look like this:
//
//   #include ...
//   #include ...
//   #include ...
//   #define MICRO_BENCH_IMPLEMENTATION
//   #include "micro-bench.h"
//
// Code
// ----
//
// The official git repository of micro-bench.h is hosted at:
//
//     https://github.com/San7o/micro-bench.h
//
// This is part of a bigger collection of header-only C99 libraries
// called "micro-headers", contributions are welcome:
//
//     https://github.com/San7o/micro-headers
//

#ifndef _MICRO_BENCH_H_
#define _MICRO_BENCH_H_

#define MICRO_BENCH_MAJOR 0
#define MICRO_BENCH_MINOR 1

#ifdef __cplusplus
extern "C" {
#endif

//
// Types
//

#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 199309L
#endif
#ifdef _WIN32
#error "TODO: support for windows clock"
#endif
  
#include <time.h>

// Data recorded
//  
// This is updated each time the start and stop functions are called
typedef struct {
  double min_cpu, min_real;
  double max_cpu, max_real;
  double sum_cpu, sum_real;
  double mean_cpu, mean_real;
  // running sum of squared deviations from the mean. Used in
  // Welford's online algorithm to calculate variance
  double M2_cpu, M2_real;
  double variance_cpu, variance_real;
  long unsigned int iterations;
} MicroBenchData;

// A reporter handles output of data
//
// You can define your own reporter function and use it with
// `micro_bench_report_with`. A few reported are provided by default
//(see below).
typedef void (*MicroBenchReporter)(MicroBenchData *data);

// A micro benchmark
typedef struct {
  MicroBenchData data;
  clock_t start_time_cpu;
  struct timespec start_time_real;
} MicroBench;

//
// Function declarations
//

// Start a benchmark
//
// Time data will be recorded and saved internally.
// Note: you need to call `micro_bench_stop` to end it.
void micro_bench_start(MicroBench *mb);

// Stop a benchmark  
void micro_bench_stop(MicroBench *mb);

// Reset internal benchmark data captured so far
void micro_bench_clear(MicroBench *mb);

// Getters for either real time and cpu time
double micro_bench_get_min_real(MicroBench *mb);
double micro_bench_get_min_cpu(MicroBench *mb);
double micro_bench_get_max_real(MicroBench *mb);
double micro_bench_get_max_cpu(MicroBench *mb);
double micro_bench_get_mean_real(MicroBench *mb);
double micro_bench_get_mean_cpu(MicroBench *mb);
double micro_bench_get_sum_real(MicroBench *mb);
double micro_bench_get_sum_cpu(MicroBench *mb);
double micro_bench_get_variance_real(MicroBench *mb);
double micro_bench_get_variance_cpu(MicroBench *mb);

// Print recorded information to stdout in a nice box
void micro_bench_default_reporter_stdout(MicroBenchData *data);
// Print recorded information to stout as CSV
void micro_bench_default_reporter_csv(MicroBenchData *data);

// Report benchmark data with the default stdout reporter
void micro_bench_report(MicroBench *mb);
// Report benchmark data with a specific [reporter]
void micro_bench_report_with(MicroBench *mb,
                             MicroBenchReporter reporter);

//
// Implementation
//
  
#ifdef MICRO_BENCH_IMPLEMENTATION

#include <stdio.h>

void micro_bench_start(MicroBench *mb)
{
  if (!mb) return;
  mb->start_time_cpu = clock();
  clock_gettime(CLOCK_MONOTONIC, &mb->start_time_real);
  return;
}

void micro_bench_stop(MicroBench *mb)
{
  if (!mb) return;
  
  double stop_time_cpu = clock();
  struct timespec stop_time_real;
  clock_gettime(CLOCK_MONOTONIC, &stop_time_real);
  double diff_cpu = (double)(stop_time_cpu - mb->start_time_cpu) / CLOCKS_PER_SEC;
  double diff_real = (stop_time_real.tv_sec - mb->start_time_real.tv_sec)
    + (stop_time_real.tv_nsec - mb->start_time_real.tv_nsec) / 1e9;

  if (diff_cpu < mb->data.min_cpu || mb->data.min_cpu == 0.0)
    mb->data.min_cpu = diff_cpu;
  if (diff_real < mb->data.min_real || mb->data.min_real == 0.0)
    mb->data.min_real = diff_real;
  if (diff_cpu > mb->data.max_cpu)
    mb->data.max_cpu = diff_cpu;
  if (diff_real > mb->data.max_real)
    mb->data.max_real = diff_real;

  mb->data.sum_cpu += diff_cpu;
  mb->data.sum_real += diff_real;
  mb->data.iterations++;

  // Welford's online algorithm to calculate variance
  double delta_cpu = diff_cpu - mb->data.mean_cpu;
  mb->data.mean_cpu += delta_cpu / mb->data.iterations;
  double delta2_cpu = diff_cpu - mb->data.mean_cpu;
  mb->data.M2_cpu += delta_cpu * delta2_cpu;
  mb->data.variance_cpu = mb->data.M2_cpu / mb->data.iterations;
  
  double delta_real = diff_real - mb->data.mean_real;
  mb->data.mean_real += delta_real / mb->data.iterations;
  double delta2_real = diff_real - mb->data.mean_real;
  mb->data.M2_real += delta_real * delta2_real;
  mb->data.variance_real = mb->data.M2_real / mb->data.iterations;
  
  return;
}

void micro_bench_clear(MicroBench *mb)
{
  if (!mb) return;
  mb->data = (MicroBenchData){0};
  mb->start_time_cpu = (clock_t){0};
  mb->start_time_real = (struct timespec){0};
  return;
}

  
double micro_bench_get_min_real(MicroBench *mb)
{
  return mb->data.min_real;
}
  
double micro_bench_get_min_cpu(MicroBench *mb)
{
  return mb->data.min_cpu;
}

double micro_bench_get_max_real(MicroBench *mb)
{
  return mb->data.max_real;
}
  
double micro_bench_get_max_cpu(MicroBench *mb)
{
  return mb->data.max_cpu;
}

double micro_bench_get_mean_real(MicroBench *mb)
{
  return mb->data.mean_real;
}

double micro_bench_get_mean_cpu(MicroBench *mb)
{
  return mb->data.mean_cpu;
}

double micro_bench_get_sum_real(MicroBench *mb)
{
  return mb->data.max_real;
}

double micro_bench_get_sum_cpu(MicroBench *mb)
{
  return mb->data.max_cpu;
}

double micro_bench_get_variance_real(MicroBench *mb)
{
  return mb->data.variance_real;
}

double micro_bench_get_variance_cpu(MicroBench *mb)
{
  return mb->data.variance_cpu;
}

void micro_bench_default_reporter_stdout(MicroBenchData *data)
{
  printf("\n");
  printf("/---------------------------------------\\\n");
  printf("|         Micro benchmark report        |\n");
  printf("|---------------------------------------|\n");
  printf("|   ////   |     real     |     CPU     |\n");
  printf("|---------------------------------------|\n");
  printf("|   min    |  %1.7f   |  %1.7f  |\n", data->min_real, data->min_cpu);
  printf("|   max    |  %1.7f   |  %1.7f  |\n", data->max_real, data->max_cpu);
  printf("|   sum    |  %1.7f   |  %1.7f  |\n", data->sum_real, data->sum_cpu);
  printf("|   mean   |  %1.7f   |  %1.7f  |\n", data->mean_real, data->mean_cpu);
  printf("|   var    |  %1.7f   |  %1.7f  |\n", data->variance_real, data->variance_cpu);
  printf("|---------------------------------------|\n");
  printf("|   iterations   |    %9ld         |\n", data->iterations);
  printf("\\---------------------------------------/\n");
  return;
}

void micro_bench_default_reporter_csv(MicroBenchData *data)
{
  printf("min_real,min_cpu,sum_real,sum_cpu,mean_real,mean_cpu,variance_real,variance_cpu,iterations\n");
  printf("%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%ld\n",
         data->min_real, data->min_cpu, data->max_real, data->max_cpu,
         data->sum_real, data->sum_cpu, data->mean_real, data->mean_cpu,
         data->variance_real, data->variance_cpu, data->iterations);
  return;
}
  
void micro_bench_report(MicroBench *mb)
{
  micro_bench_report_with(mb, micro_bench_default_reporter_stdout);
  return;
}

void micro_bench_report_with(MicroBench *mb,
                             MicroBenchReporter reporter)
{
  reporter(&mb->data);
  return;
}

#endif // MICRO_BENCH_IMPLEMENTATION

//
// Examples
//
  
#if 0

#define MICRO_BENCH_IMPLEMENTATION
#include "micro-bench.h"

int fib(int x) {
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
    fib(35);
    micro_bench_stop(&mb);
  }

  micro_bench_report(&mb);
  
  // or use a specific reporter
  printf("\nCSV exporter:\n");
  micro_bench_report_with(&mb, micro_bench_default_reporter_csv);
  
  return 0;
}

#endif // 0
  
#ifdef __cplusplus
}
#endif

#endif // _MICRO_BENCH_H_
