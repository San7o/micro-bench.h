//////////////////////////////////////////////////////////////////////
// SPDX-License-Identifier: MIT
//
// micro-bench.h
// -------------
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
// /------------------------\
// | Micro benchmark report |
// |------------------------|
// |   min    |  0.0626740  |
// |   max    |  0.0746470  |
// |   sum    |  0.6480160  |
// |   mean   |  0.0648016  |
// |   var    |  0.0000125  |
// |   it     |         10  |
// \------------------------/
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

#include <time.h>

// Data recorded
//  
// This is updated each time the start and stop functions are called
typedef struct {
  double min;
  double max;
  double sum;
  double mean;
  // running sum of squared deviations from the mean. Used in
  // Welford's online algorithm to calculate variance
  double M2;
  double variance;
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
  clock_t start_time;
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

double micro_bench_get_min(MicroBench *mb);
double micro_bench_get_max(MicroBench *mb);
double micro_bench_get_mean(MicroBench *mb);
double micro_bench_get_sum(MicroBench *mb);
double micro_bench_get_variance(MicroBench *mb);

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
  mb->start_time = clock();
  return;
}

void micro_bench_stop(MicroBench *mb)
{
  if (!mb) return;
  
  double stop_time = clock();
  double diff = (double)(stop_time - mb->start_time) / CLOCKS_PER_SEC;

  if (diff < mb->data.min || mb->data.min == 0.0)
    mb->data.min = diff;
  if (diff > mb->data.max)
    mb->data.max = diff;

  mb->data.sum += diff;
  mb->data.iterations++;

  // Welford's online algorithm to calculate variance
  double delta = diff - mb->data.mean;
  mb->data.mean += delta / mb->data.iterations;
  double delta2 = diff - mb->data.mean;
  mb->data.M2 += delta * delta2;
  mb->data.variance = mb->data.M2 / mb->data.iterations;   
  return;
}

void micro_bench_clear(MicroBench *mb)
{
  if (!mb) return;
  mb->data = (MicroBenchData){0};
  mb->start_time = (clock_t){0};
  return;
}

double micro_bench_get_min(MicroBench *mb)
{
  return mb->data.min;
}

double micro_bench_get_max(MicroBench *mb)
{
  return mb->data.max;
}

double micro_bench_get_mean(MicroBench *mb)
{
  return mb->data.mean;
}

double micro_bench_get_sum(MicroBench *mb)
{
  return mb->data.max;
}

double micro_bench_get_variance(MicroBench *mb)
{
  return mb->data.variance;
}

void micro_bench_default_reporter_stdout(MicroBenchData *data)
{
  printf("\n");
  printf("/------------------------\\\n");
  printf("| Micro benchmark report |\n");
  printf("|------------------------|\n");
  printf("|   min    |  %1.7f  |\n", data->min);
  printf("|   max    |  %1.7f  |\n", data->max);
  printf("|   sum    |  %1.7f  |\n", data->sum);
  printf("|   mean   |  %1.7f  |\n", data->mean);
  printf("|   var    |  %1.7f  |\n", data->variance);
  printf("|   it     |  %9ld  |\n", data->iterations);
  printf("\\------------------------/\n");
  return;
}

void micro_bench_default_reporter_csv(MicroBenchData *data)
{
  printf("%f,%f,%f,%f,%f,%ld\n", data->min, data->max, data->sum,
         data->mean, data->variance, data->iterations);
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
  for (int i = 0; i < 10; ++i)
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
