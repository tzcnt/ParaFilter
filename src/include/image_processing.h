#pragma once
#include <map>
#include <vector>
#include <omp.h>
#include "image.h"

/**
 * @enum Filter
 * @brief Enum class to maintain a strong type for different types of filters.
 */
enum class Filter {
  LowPass3x3,  ///< Low pass filter with a 3x3 kernel.
  LowPass5x5,  ///< Low pass filter with a 5x5 kernel.
  HighPass3x3, ///< High pass filter with a 3x3 kernel.
  HighPass5x5, ///< High pass filter with a 5x5 kernel.
  Gaussian     ///< Gaussian blur filter.
};

/**
 * @typedef Kernel
 * @brief Typedef for a 2D vector of floats to represent the convolution kernel matrix.
 */
using Kernel = std::vector<std::vector<float>>;

/**
 * @var kernels
 * @brief Map to store various predefined kernels, accessible by filter type.
 */
extern std::map<Filter, Kernel> kernels;
Image applyKernelSeq(Image &img, const Kernel &kernel);
Image applyKernelOpenMp(Image &img, const Kernel &kernel, int nthreads);
Image applyKernelMPI(Image &img, const Kernel &kernel);
