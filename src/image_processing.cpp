#include "include/image_processing.h"
#include "include/image.h"
#include <iostream>
#include <iterator>
#include <omp.h>
#include <ostream>

template <typename T> T clamp(T value, T min, T max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  }
  return value;
}

std::map<Filter, Kernel> kernels =
    {{Filter::LowPass3x3,
      {{1.0f / 9, 1.0f / 9, 1.0f / 9},
       {1.0f / 9, 1.0f / 9, 1.0f / 9},
       {1.0f / 9, 1.0f / 9, 1.0f / 9}}},
     {Filter::LowPass5x5,
      {{1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
       {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
       {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
       {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
       {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25}}},
     {Filter::HighPass3x3,
      {{-1.0f / 4, -1.0f / 4, -1.0f / 4},
       {-1.0f / 4, 2.0f, -1.0f / 4},
       {-1.0f / 4, -1.0f / 4, -1.0f / 4}}},
     {Filter::HighPass5x5,
      {{-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
       {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
       {-1 / 25.0, -1 / 25.0, 24, -1 / 25.0, -1 / 25.0},
       {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
       {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0}}},
     {Filter::Gaussian,
      {{1.0f / 16, 2.0f / 16, 1.0f / 16},
       {2.0f / 16, 4.0f / 16, 2.0f / 16},
       {1.0f / 16, 2.0f / 16, 1.0f / 16}}}}

;

Image applyKernelSeq(Image &img, const Kernel &kernel) {
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Create output image array
  unsigned char *output =
      new unsigned char[img.width * img.height * img.channels];

  // Loop over each pixel in the image
  for (int y = kHalf; y < img.height - kHalf; y++) {
    for (int x = kHalf; x < img.width - kHalf; x++) {
      std::vector<float> sum(img.channels, 0.0f);

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
 * @brief Applies a convolution kernel to an input image to produce an output image.
 *
 * This function takes an input image and a convolution kernel, applies the
 * convolution operation pixel by pixel by using the kernel, and produces the
 * output image.
 *
 * @param input Pointer to the input image data (continuous memory block).
 * @param output Pointer to the output image data where the result should be stored.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param channels The number of channels in the image (e.g., 3 for RGB, 1 for grayscale).
 * @param kernel The convolution kernel as a 2D vector of floats. The kernel
 * should be a square matrix (n x n) and usually has odd dimensions (3x3, 5x5, etc.).
 */

Image applyKernelOpenMp(Image &img, const Kernel &kernel) {
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Create output image array
  unsigned char *output =
      new unsigned char[img.width * img.height * img.channels];

  omp_set_num_threads(8);

  // Loop over each pixel in the image
#pragma omp parallel for collapse(2)
  for (int y = kHalf; y < img.height - kHalf; y++) {
    for (int x = kHalf; x < img.width - kHalf; x++) {
      std::vector<float> sum(img.channels, 0.0f);

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

Image applyKernelMPI(Image &img, const Kernel &kernel) {
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Create output image array
  unsigned char *output =
      new unsigned char[img.width * img.height * img.channels];

  // Loop over each pixel in the image
  for (int y = kHalf; y < img.height - kHalf; y++) {
    for (int x = kHalf; x < img.width - kHalf; x++) {
      std::vector<float> sum(img.channels, 0.0f);

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
