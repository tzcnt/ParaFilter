#include <cstdlib>
#include <iostream>
#include "include/image.h"
#include "include/stb_image_write.h"
#include "include/image_processing.h"

using namespace std;

/**
 * @function main
 * @brief Main entry point of the application.
 * @returns Exit status of the application.
 */
int main() {
  int width, height, channels;
  const char *inputFile = "./examples/tiger.jpg";
  const char *outputFile = "./outputs/tiger_modified.jpg";

  // Load image
  try {
    Image img = Image::load(inputFile);
    cout << "Loaded image with dimensions: " << img.width << "x" << img.height
         << ", Channels: " << img.channels << endl;
    Kernel kernel = kernels[Filter::HighPass5x5];
    img.pad(kernel.size() / 2);
    cout << "image after padding: " << img.width << "x" << img.height
         << ", Channels: " << img.channels << endl;

    Image outputImage = applyKernelOpenMp(img, kernel);

    if (!outputImage.save(outputFile, "jpg")) {
      cerr << "Failed to save image as jpg." << endl;
    }
    cout << "Filtering completed and image saved to: " << outputFile << endl;
  } catch (const runtime_error &e) {
    cerr << "Error: " << e.what() << endl;
  }
  return EXIT_SUCCESS;
}
