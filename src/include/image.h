#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include "stb_image.h"

void image_data_deleter(unsigned char *p);

struct Image {
  int width, height, channels;
  std::unique_ptr<unsigned char, decltype(&image_data_deleter)> data;

  Image()
      : width(0), height(0), channels(0), data(nullptr, &image_data_deleter) {}

  Image(unsigned char *data, int width, int height, int channels)
      : width(width), height(height), channels(channels),
        data(data, image_data_deleter) {}

  static Image load(const char *filename) {
    int width, height, channels;
    unsigned char *raw_data =
        stbi_load(filename, &width, &height, &channels, 0);
    if (!raw_data) {
      throw std::runtime_error(stbi_failure_reason());
    }
    return Image(raw_data, width, height, channels);
  };

  bool save(const char *filename, const char *format);

  bool padZeros(int borderSize);
  bool padReplication(int borderSize);
};
