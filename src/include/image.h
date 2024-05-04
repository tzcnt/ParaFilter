#pragma once
#include <memory>
#include <stdexcept>
#include <string>
#include <cstdlib>
#include "stb_image.h"

void image_data_deleter(unsigned char *p);

struct Pixel {
  unsigned char r, g, b, a;

  Pixel() : r(0), g(0), b(0), a(255) {} // Default to opaque black
  Pixel(unsigned char r, unsigned char g, unsigned char b,
        unsigned char a = 255)
      : r(r), g(g), b(b), a(a) {}
};

struct Image {
  int width, height, channels;
  std::unique_ptr<unsigned char, decltype(&image_data_deleter)> data;

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

  struct Iterator {
    unsigned char *ptr;
    int step;

    Iterator(unsigned char *ptr, int step) : ptr(ptr), step(step) {}

    Iterator &operator++() {
      ptr += step;
      return *this;
    }

    bool operator!=(const Iterator &other) const { return ptr != other.ptr; }

    unsigned char *operator*() { return ptr; }
  };

  Iterator begin() { return Iterator(data.get(), channels); }

  Iterator end() {
    return Iterator(data.get() + width * height * channels, channels);
  }

  bool save(const char *filename, const char *format);

  bool pad(int borderSize);
};
