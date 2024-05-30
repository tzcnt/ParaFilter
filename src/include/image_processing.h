#pragma once
#include <map>
#include <vector>
#include <omp.h>
#include "image.h"
#include <algorithm>
#include <cstring>
#include <iostream>
#include <array>

// /**
//  * @enum Filter
//  * @brief Enum class to maintain a strong type for different types of filters.
//  */
// enum class Filter {
//   LowPass3x3 = 0,  ///< Low pass filter with a 3x3 kernel.
//   LowPass5x5 = 1,  ///< Low pass filter with a 5x5 kernel.
//   HighPass3x3 = 2, ///< High pass filter with a 3x3 kernel.
//   Gaussian = 3     ///< Gaussian blur filter.
// };

template <typename T> T clamp(T value, T min, T max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  }
  return value;
}
namespace Kernels {
namespace Filter {
constexpr inline std::array<std::array<float, 3>, 3> LowPass3x3() {
  return {{{1.0f / 9, 1.0f / 9, 1.0f / 9},
           {1.0f / 9, 1.0f / 9, 1.0f / 9},
           {1.0f / 9, 1.0f / 9, 1.0f / 9}}};
};

constexpr inline std::array<std::array<float, 3>, 3> HighPass3x3() {
  return {{{-1.0f / 4, -1.0f / 4, -1.0f / 4},
           {-1.0f / 4, 2.0f, -1.0f / 4},
           {-1.0f / 4, -1.0f / 4, -1.0f / 4}}};
};

constexpr inline std::array<std::array<float, 3>, 3> Gaussian() {
  return {{{1.0f / 16, 2.0f / 16, 1.0f / 16},
           {2.0f / 16, 4.0f / 16, 2.0f / 16},
           {1.0f / 16, 2.0f / 16, 1.0f / 16}}};
};

constexpr inline std::array<std::array<float, 5>, 5> LowPass5x5() {
  return {{{1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
           {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
           {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
           {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
           {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25}}};
};
} // namespace Filter
} // namespace Kernels

/**
 * @brief Applies a convolution kernel to an input image to produce an output image but uses OpenMP.
 */
template <typename Kernel>
Image applyKernelSeq(Image &img, const Kernel kernel) {
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Create output image array
  unsigned char *output =
      new unsigned char[img.width * img.height * img.channels];

  memcpy(output, img.data.get(), img.width * img.height * img.channels);

  std::vector<float> sum(img.channels, 0.0f);
  // Loop over each pixel in the image
  for (int y = kHalf; y < img.height - kHalf; y++) {
    for (int x = kHalf; x < img.width - kHalf; x++) {
      for (int c = 0; c < img.channels; c++) {
        sum[c] = 0.0f;
      }

      // Perform the convolution operation
      for (int ky = -kHalf; ky <= kHalf; ky++) {
        for (int kx = -kHalf; kx <= kHalf; kx++) {

          int px = (x + kx);
          int py = (y + ky);
          const unsigned char *pixel =
              img.data.get() + (py * img.width + px) * img.channels;

          for (int c = 0; c < img.channels; c++) {
            sum[c] += pixel[c] * kernel[ky + kHalf][kx + kHalf];
          }
        }
      }
      unsigned char *outPixel = output + (y * img.width + x) * img.channels;
      for (int c = 0; c < img.channels; c++) {
        // Clamp the values to the range [0, 255]
        outPixel[c] = static_cast<unsigned char>(clamp((int)sum[c], 0, 255));
      }
      const unsigned char *pixel =
          img.data.get() + (y * img.width + x) * img.channels;
      outPixel[3] = pixel[3];
    }
  }
  return Image(output, img.width, img.height, img.channels);
}

/**
 * @brief Applies a convolution kernel to an input image to produce an output image but uses OpenMP.
 */
#ifdef OPENMP
template <typename Kernel>
Image applyKernelOpenMp(Image &img, const Kernel kernel, int nthreads) {
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Create output image array
  unsigned char *output =
      new unsigned char[img.width * img.height * img.channels];

  memcpy(output, img.data.get(), img.width * img.height * img.channels);

  omp_set_num_threads(nthreads);

  // Loop over each pixel in the image
#pragma omp parallel
  {
    std::vector<float> sum(img.channels, 0.0f);
#pragma omp for collapse(2)
    for (int y = kHalf; y < img.height - kHalf; y++) {
      for (int x = kHalf; x < img.width - kHalf; x++) {
        for (int c = 0; c < img.channels; c++) {
          sum[c] = 0.0f;
        }

        // Perform the convolution operation
        for (int ky = -kHalf; ky <= kHalf; ky++) {
          for (int kx = -kHalf; kx <= kHalf; kx++) {

            int px = (x + kx);
            int py = (y + ky);
            const unsigned char *pixel =
                img.data.get() + (py * img.width + px) * img.channels;

            for (int c = 0; c < img.channels; c++) {
              sum[c] += pixel[c] * kernel[ky + kHalf][kx + kHalf];
            }
          }
        }
        const unsigned char *pixel =
            img.data.get() + (y * img.width + x) * img.channels;
        unsigned char *outPixel = output + (y * img.width + x) * img.channels;
        for (int c = 0; c < img.channels; c++) {
          outPixel[c] = static_cast<unsigned char>(clamp((int)sum[c], 0, 255));
        }
        outPixel[3] = pixel[3];
      }
    }
  }
  return Image(output, img.width, img.height, img.channels);
}
#endif
