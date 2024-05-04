#include "include/image.h"
#include <cstdint>
#include <iostream>
#include <iterator>
#include <ostream>
#define STB_IMAGE_IMPLEMENTATION
#include "include/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "include/stb_image_write.h"

void image_data_deleter(unsigned char *p) { stbi_image_free(p); }

bool Image::save(const char *filename, const char *format) {
  if (std::string(format) == "png") {
    return stbi_write_png(filename, this->width, height, channels, data.get(),
                          this->width * channels);
  } else if (std::string(format) == "jpg") {
    return stbi_write_jpg(filename, this->width, height, channels, data.get(),
                          100); // 100 is the quality parameter
  } else if (std::string(format) == "bmp") {
    return stbi_write_bmp(filename, this->width, height, channels, data.get());
  } else if (std::string(format) == "tga") {
    return stbi_write_tga(filename, width, height, channels, data.get());
  }
  return false; // Unsupported format
}

bool Image::pad(int borderSize) {
  int newWidth = width + 2 * borderSize;
  int newHeight = height + 2 * borderSize;
  std::unique_ptr<unsigned char, decltype(&image_data_deleter)> newData(
      new unsigned char[newWidth * newHeight * channels], image_data_deleter);

  // Initialize new image data with padValue
  memset(newData.get(), 0, newWidth * newHeight * channels);

  // Copy original image data to the center of the new image
  for (int y = 0; y < height; ++y) {
    unsigned char *src = data.get() + y * width * channels;
    unsigned char *dst = newData.get() +
                         (y + borderSize) * newWidth * channels +
                         borderSize * channels;
    memcpy(dst, src, width * channels);
  }

  // Replace the old data with the new padded data
  width = newWidth;
  height = newHeight;
  data.swap(newData);

  return true;
}
