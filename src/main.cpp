#include <cstdlib>
#include <mpi.h>
#include <iostream>
#include "include/image.h"
#include "include/stb_image_write.h"
#include "include/image_processing.h"

using namespace std;

#define USE_MPI 1

int main(int argc, char *argv[]) {
  int width, height, channels;
  const char *tigerFile = "./examples/tiger.jpg";
  const char *tigerOutputFile = "./outputs/tiger_modified.jpg";

  const char *lenaFile = "./examples/lena.png";
  const char *lenaOutputFile = "./outputs/lena_modified.png";

#if USE_MPI
  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Image img;
  Kernel kernel = kernels[Filter::HighPass3x3];

  if (rank == 0) {
    try {
      img = Image::load(lenaFile);
      img.pad(kernel.size() / 2);
    } catch (const std::runtime_error &e) {
      cerr << "Failed to load image: " << e.what() << std::endl;
      MPI_Abort(MPI_COMM_WORLD, 1);
    }
  }

  // Broadcast image dimensions and channels to all processes
  MPI_Bcast(&img.width, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&img.height, 1, MPI_INT, 0, MPI_COMM_WORLD);
  MPI_Bcast(&img.channels, 1, MPI_INT, 0, MPI_COMM_WORLD);

  int extra = kernel.size() / 2;

  // Calculate the stripe height per process
  int rows_per_process = img.height / size;
  int num_elements = (rows_per_process + extra) * img.width * img.channels;

  // Allocate memory for each sub-image
  unsigned char *sub_image_data = new unsigned char[num_elements];

  // Scatter the image data
  MPI_Scatter(img.data.get(), num_elements, MPI_UNSIGNED_CHAR, sub_image_data,
              num_elements, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  Image local_sub_image(sub_image_data, img.width, (rows_per_process + extra),
                        img.channels);
  Image processed_sub_image = applyKernelMPI(local_sub_image, kernel);

  // Gather processed sub-images back to the root
  unsigned char *output_data = nullptr;
  if (rank == 0) {
    output_data = new unsigned char[img.width * img.height * img.channels];
  }

  num_elements = processed_sub_image.width *
                 (processed_sub_image.height - extra) *
                 processed_sub_image.channels;

  MPI_Gather(processed_sub_image.data.get(), num_elements, MPI_UNSIGNED_CHAR,
             output_data, num_elements, MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  if (rank == 0) {
    Image finalOutput = Image(output_data, img.width,
                              img.height - (extra * size), img.channels);
    finalOutput.save(lenaOutputFile, "png");
  }

  MPI_Finalize();
  return 0;
#else
  // Load image
  try {
    Image img = Image::load(lenaFile);
    Kernel kernel = kernels[Filter::HighPass3x3];
    img.pad(kernel.size() / 2);

    Image outputImage = applyKernelOpenMp(img, kernel, 1);

    if (!outputImage.save(lenaOutputFile, "png")) {
      cerr << "Failed to save image as jpg." << endl;
    }
    cout << "Filtering completed and image saved to: " << tigerOutputFile
         << endl;
  } catch (const runtime_error &e) {
    cerr << "Error: " << e.what() << endl;
  }

#endif
  return EXIT_SUCCESS;
}
