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
  std::vector<unsigned char> testImage(width * height * channels, 10);
  testImage[12] = 50;

  std::vector<unsigned char> outputImage(width * height * channels, 0);

  Kernel kernel = kernels[Filter::LowPass3x3];
  applyKernel(testImage.data(), outputImage.data(), width, height, channels,
              kernel);

  // Expected output should show the averaging effect, but we ignore the edges
  int expectedCenterValue = (10 + 10 + 10 + 10 + 50 + 10 + 10 + 10 + 10) / 9;
  for (int y = 1; y < height - 1; y++) {  // Ignore the top and bottom rows
    for (int x = 1; x < width - 1; x++) { // Ignore the left and right columns
      int index = y * width + x;
      EXPECT_EQ(outputImage[index], expectedCenterValue)
          << "Pixel index " << index << " did not match expected output.";
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
