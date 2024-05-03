#include <cstdlib>
#include <iostream>
#include <vector>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/image_processing.h"
#include "include/stb_image_write.h"

using namespace std;

/**
 * @function main
 * @brief Main entry point of the application.
 * @returns Exit status of the application.
 */
int main() {
  int width, height, channels;
  const char *inputFile = "./examples/lena.png";
  const char *outputFile = "./outputs/output.jpg";

  // Load image
  unsigned char *inputImage =
      stbi_load(inputFile, &width, &height, &channels, 0);
  if (inputImage == nullptr) {
    cerr << "Error in loading the image" << endl;
    return EXIT_FAILURE;
  }

  // Create output image array
  vector<unsigned char> outputImage(width * height * channels);

  Kernel kernel = kernels[Filter::HighPass5x5];

  // Apply low pass filter
  applyKernel(inputImage, outputImage.data(), width, height, channels, kernel);

  // Save the filtered image
  if (!stbi_write_jpg(outputFile, width, height, channels, outputImage.data(),
                      100)) {
    cerr << "Failed to write the image" << endl;
    stbi_image_free(inputImage);
    return EXIT_FAILURE;
  }

  // Clean up resources not tracked by RAII
  stbi_image_free(inputImage);
  cout << "Filtering completed and image saved to: " << outputFile << endl;

  return EXIT_SUCCESS;
}
