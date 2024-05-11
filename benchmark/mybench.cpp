#include <benchmark/benchmark.h>
#include "../src/include/image_processing.h"

static void BM_Sequential(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;
  const char *inputFile = "./examples/lena.png";
  const char *outputFile = "./outputs/lena_modified.png";

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::LowPass3x3];
  img.padReplication(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelSeq(img, kernel);
  }
}
static void BM_OpenMP(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;
  auto nthreads = state.range(0);
  const char *inputFile = "./examples/lena.png";
  const char *outputFile = "./outputs/lena_modified.png";

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::LowPass3x3];
  img.padReplication(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelOpenMp(img, kernel, nthreads);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_Sequential)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_OpenMP)->DenseRange(4, 32, 4)->Unit(benchmark::kMillisecond);
// Run the benchmark
BENCHMARK_MAIN();
