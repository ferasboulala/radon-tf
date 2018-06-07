#include "radon.hpp"
#include <iostream>
#include <string>
#include <chrono>

int main(int argc, char** argv){
  std::cout << "Radon transform test program" << std::endl;
  if (argc != 3){
    std::cout << "Please provide the image filename (exiting)" << std::endl;
    return -1;
  }
  std::string filename = argv[1];
  int n_threads = atoi(argv[2]);
  cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
  cv::resize(img, img, cv::Size(360,360));
  std::chrono::high_resolution_clock::time_point start, end;
	std::chrono::duration<double, std::milli> fp_ms;
  start = std::chrono::high_resolution_clock::now();
  cv::Mat rad = cv::sinogram(img, 360, n_threads);
  end = std::chrono::high_resolution_clock::now();
  fp_ms = end - start;
  std::cout << "Radon transform with " << n_threads << " threads took "
    << fp_ms.count() << " ms" << std::endl;

  cv::Mat compare(img.rows * 2, img.cols * 2, img.type());
  cv::hconcat(img, rad, compare);

  cv::namedWindow("Sinogram", cv::WINDOW_NORMAL);
  cv::imshow("Sinogram", compare);
  cv::waitKey(0);
  return 0;
}
