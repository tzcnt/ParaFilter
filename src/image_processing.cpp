#include "include/image_processing.h"

template <typename T> T clamp(T value, T min, T max) {
  if (value < min) {
    return min;
  } else if (value > max) {
    return max;
  }
  return value;
}

std::map<Filter, Kernel> kernels = {
    {Filter::LowPass3x3,
     {{1.0f / 9, 1.0f / 9, 1.0f / 9},
      {1.0f / 9, 1.0f / 9, 1.0f / 9},
      {1.0f / 9, 1.0f / 9, 1.0f / 9}}},
    {Filter::LowPass5x5,
     {{1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
      {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
      {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
      {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25},
      {1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25, 1.0f / 25}}},
    {Filter::HighPass3x3, {{-1, -1, -1}, {-1, 8, -1}, {-1, -1, -1}}},
    {Filter::HighPass5x5,
     {{-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
      {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
      {-1 / 25.0, -1 / 25.0, 24 / 25.0, -1 / 25.0, -1 / 25.0},
      {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0},
      {-1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0, -1 / 25.0}}},
    {Filter::Gaussian,
     {{1.0f / 16, 2.0f / 16, 1.0f / 16},
      {2.0f / 16, 4.0f / 16, 2.0f / 16},
      {1.0f / 16, 2.0f / 16, 1.0f / 16}}}};

/**
 * @brief Applies a convolution kernel to an input image to produce an output
 * image.
 *
 * This function takes an input image and a convolution kernel, applies the
 * convolution operation pixel by pixel by using the kernel, and produces the
 * output image. This process is used in image processing for effects such as
 * blurring, sharpening, and edge detection.
 *
 * @param input Pointer to the input image data (continuous memory block).
 * @param output Pointer to the output image data where the result should be
 * stored.
 * @param width The width of the image in pixels.
 * @param height The height of the image in pixels.
 * @param channels The number of channels in the image (e.g., 3 for RGB, 1 for
 * grayscale).
 * @param kernel The convolution kernel as a 2D vector of floats. The kernel
 * should be a square matrix (n x n) and usually has odd dimensions (3x3, 5x5,
 * etc.).
 */
void applyKernel(const unsigned char *input, unsigned char *output, int width,
                 int height, int channels, const Kernel &kernel) {
  // Get the size of the kernel and compute the half-size for boundary
  // calculations.
  int kernelSize = kernel.size();
  int kHalf = kernelSize / 2;

  // Loop over each pixel in the image, avoiding the border as defined by kernel
  // half-size.
  for (int y = kHalf; y < height - kHalf; y++) {
    for (int x = kHalf; x < width - kHalf; x++) {
      // Initialize a sum vector to accumulate the new values for each channel.
      std::vector<float> sum(channels, 0.0f);

      // Perform the convolution operation: iterate over the kernel's coverage.
      for (int ky = -kHalf; ky <= kHalf; ky++) {
        for (int kx = -kHalf; kx <= kHalf; kx++) {
          // Calculate the actual pixel position by offsetting from the current
          // pixel.
          int px = (x + kx);
          int py = (y + ky);
          // Access the pixel value from the input image.
          const unsigned char *pixel = input + (py * width + px) * channels;

          // Apply the kernel to each channel of the pixel.
          for (int c = 0; c < channels; c++) {
            sum[c] += pixel[c] * kernel[ky + kHalf][kx + kHalf];
          }
        }
      }

      // Store the computed values back into the output image.
      unsigned char *outPixel = output + (y * width + x) * channels;
      for (int c = 0; c < channels; c++) {
        // Clamp the values to the range [0, 255] and cast to unsigned char before saving.
        outPixel[c] = static_cast<unsigned char>(clamp(int(sum[c]), 0, 255));
      }
    }
  }
}
