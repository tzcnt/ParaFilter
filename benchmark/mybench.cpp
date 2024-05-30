#define TMC_IMPL

#include "tmc/all_headers.hpp"
#include <benchmark/benchmark.h>

#include "../src/include/image_processing.h"

static const char *inputFile = "./4k_wallpaper.jpg";
static const char *outputFile = "./lena_modified.png";
static void BM_Sequential(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::LowPass3x3];
  img.padReplication(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelSeq(img, kernel);
  }
}

// static void BM_OpenMP(benchmark::State &state) {
//   // Perform setup here
//   int width, height, channels;
//   auto nthreads = state.range(0);

//   // Load image
//   Image img = Image::load(inputFile);
//   Kernel kernel = kernels[Filter::LowPass3x3];
//   img.padReplication(kernel.size() / 2);
//   for (auto _ : state) {
//     Image outputImage = applyKernelOpenMp(img, kernel, nthreads);
//   }
// }

static void BM_TooManyCooks(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;
  auto nthreads = state.range(0);
  tmc::cpu_executor().set_thread_count(nthreads).init();

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::LowPass3x3];
  img.padReplication(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelTooManyCooks(img, kernel);
  }
  tmc::cpu_executor().teardown();
}

// Register the function as a benchmark
BENCHMARK(BM_Sequential)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_TooManyCooks)
    ->DenseRange(4, 256, 4)
    ->Unit(benchmark::kMillisecond);
// Run the benchmark
BENCHMARK_MAIN();
