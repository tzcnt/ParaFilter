#include <cstdlib>
#ifndef USE_MPI
#include <limits>
#endif
#include <mpi.h>
#include <iostream>
#include "include/image.h"
#include "include/stb_image_write.h"
#include "include/image_processing.h"

using namespace std;

// Function to extract file extension from a file path
string getFileExtension(const string &fileName) {
  size_t dotPos = fileName.rfind('.');
  if (dotPos == string::npos) {
    return "";
  }
  return fileName.substr(dotPos + 1);
}

#ifdef USE_MPI
int main(int argc, char *argv[]) {
  const char *inputFile = "./examples/lena.png";
  const char *outputFile = "./outputs/lena_modified.png";

  MPI_Init(&argc, &argv);

  int rank, size;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  Image img;
  Kernel kernel = kernels[Filter::LowPass3x3];

  if (rank == 0) {
    try {
      img = Image::load(inputFile);
      img.padReplication(kernel.size() / 2);
    } catch (const std::runtime_error &e) {
      cerr << "Failed to load image: " << e.what() << std::endl;
      cerr << inputFile << '\n';
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
#ifdef USE_OM
  Image processed_sub_image = applyKernelOpenMp(local_sub_image, kernel);
#else
  Image processed_sub_image = applyKernelSeq(local_sub_image, kernel);
#endif

  // Gather processed sub-images back to the root
  unsigned char *output_data = nullptr;
  if (rank == 0) {
    output_data = new unsigned char[img.width * img.height * img.channels];
  }

  num_elements -= img.width * img.channels;

  MPI_Gather(processed_sub_image.data.get() + (img.width * img.channels),
             num_elements, MPI_UNSIGNED_CHAR, output_data, num_elements,
             MPI_UNSIGNED_CHAR, 0, MPI_COMM_WORLD);

  Image finalOutput = Image(output_data, img.width, img.height, img.channels);
  finalOutput.save(outputFile, "jpg");
  if (rank == 0) {
    cout << "Filtering completed and image saved to: " << outputFile << endl;
  }

  MPI_Finalize();
  return EXIT_SUCCESS;
}

#else
Kernel getCustomKernel() {
  int size;
  std::cout << "Enter the size of the kernel (n for an n x n matrix): ";
  std::cin >> size;
  Kernel customKernel(size, std::vector<float>(size));

  std::cout << "Enter the kernel values row by row:\n";
  for (int i = 0; i < size; ++i) {
    for (int j = 0; j < size; ++j) {
      std::cin >> customKernel[i][j];
    }
  }
  cin.ignore(numeric_limits<streamsize>::max(), '\n');
  return customKernel;
};

int main() {
  // Variables for file paths
  string inputFile;
  string outputFile;

  // Read file paths from user
  cout << "Enter the input image file path: ";
  getline(cin, inputFile);
  cout << "Enter the output image file path: ";
  getline(cin, outputFile);

  try {
    // Load and process the image
    Image img = Image::load(inputFile.c_str());

    int choice;
    cout << "Select a kernel option:\n";
    cout << "1. Low Pass 3x3\n";
    cout << "2. Low Pass 3x3\n";
    cout << "3. High Pass 3x3\n";
    cout << "4. Gaussian\n";
    cout << "5. Input custom kernel\n";
    cout << "Enter your choice (1-5): ";
    cin >> choice;

    cin.ignore(numeric_limits<streamsize>::max(), '\n');

    Kernel kernel;

    if (choice == 5) {
      kernel = getCustomKernel();
      // normalized the kernel
      long sum = 0;
      for (auto row : kernel) {
        for (auto x : row) {
          sum += x;
        }
      }
      for (auto &row : kernel) {
        for (auto &x : row) {
          x /= sum;
        }
      }
    } else if (choice < 5 && choice > 0) {
      kernel = kernels[static_cast<Filter>(choice - 1)];
    } else {
      std::cerr << "Invalid choice. Exiting.\n";
      return 1;
    }

    img.padReplication(kernel.size() / 2);
    Image outputImage = applyKernelOpenMp(img, kernel, 1);
    string fileExtension = getFileExtension(outputFile);

    // Save the processed image
    if (!outputImage.save(outputFile.c_str(), fileExtension.c_str())) {
      cerr << "Failed to save image as " << fileExtension << endl;
      return EXIT_FAILURE;
    }
    cout << "Filtering completed and image saved to: " << outputFile << endl;

    // Ask user if they want to open the image
    string response;
    cout << "Do you want to open the image? (y/n): ";
    getline(cin, response);
    if (response == "y" || response == "Y") {
#ifdef _WIN32
      string command = "start ";
#else
      string command = "xdg-open ";
#endif
      command += outputFile;
      system(command.c_str());
    }
  } catch (const runtime_error &e) {
    cerr << "Error: " << e.what() << endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
#endif
