#include <benchmark/benchmark.h>
#include "../src/include/image_processing.h"

static void BM_Sequential(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;
  const char *inputFile = "./examples/tiger.jpg";
  const char *outputFile = "./outputs/tiger_modified.jpg";

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::HighPass5x5];
  img.pad(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelSeq(img, kernel);
  }
}
static void BM_OpenMP(benchmark::State &state) {
  // Perform setup here
  int width, height, channels;
  auto nthreads = state.range(0);
  const char *inputFile = "./examples/tiger.jpg";
  const char *outputFile = "./outputs/tiger_modified.jpg";

  // Load image
  Image img = Image::load(inputFile);
  Kernel kernel = kernels[Filter::HighPass5x5];
  img.pad(kernel.size() / 2);
  for (auto _ : state) {
    Image outputImage = applyKernelOpenMp(img, kernel, 8);
  }
}
// Register the function as a benchmark
BENCHMARK(BM_Sequential)->Unit(benchmark::kMillisecond);
BENCHMARK(BM_OpenMP)->DenseRange(2, 16, 2)->Unit(benchmark::kMillisecond);
// Run the benchmark
BENCHMARK_MAIN();
