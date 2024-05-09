#include "../src/include/image_processing.h"
#include <gtest/gtest.h>

// 10 10 10 10 10
// 10 10 10 10 10
// 10 10 50 10 10
// 10 10 10 10 10
// 10 10 10 10 10
TEST(LowPassFilterTest, LowPass3x3) {
  int width = 5, height = 5,
      channels = 1; // Slightly larger image to have a meaningful center
  int sz = width * height * channels;
  unsigned char *testImage = new unsigned char[width * height * channels];
  memset(testImage, 10, sz);
  testImage[12] = 50;

  Image testImg = Image(testImage, width, height, channels);

  Kernel kernel = kernels[Filter::LowPass3x3];
  Image outputImage = applyKernelSeq(testImg, kernel);

  // Expected output should show the averaging effect, but we ignore the edges
  int expectedCenterValue = (10 + 10 + 10 + 10 + 50 + 10 + 10 + 10 + 10) / 9;
  for (int y = 1; y < height - 1; y++) {  // Ignore the top and bottom rows
    for (int x = 1; x < width - 1; x++) { // Ignore the left and right columns
      int index = y * width + x;
      EXPECT_EQ(outputImage.data.get()[index], expectedCenterValue)
          << "Pixel index " << index << " did not match expected output.";
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
