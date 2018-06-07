#include "radon.hpp"
#include <iostream>
#include <string>

int main(int argc, char** argv){
  std::cout << "Radon transform test program" << std::endl;
  if (argc != 2){
    throw std::invalid_argument("Please provide the image filename");
  }
  std::string filename = argv[1];
  cv::Mat img = cv::imread(filename, CV_LOAD_IMAGE_GRAYSCALE);
  cv::resize(img, img, cv::Size(360,360));
  cv::Mat rad = cv::sinogram(img, 360);

  cv::Mat compare(img.rows * 2, img.cols * 2, img.type());
  cv::hconcat(img, rad, compare);

  cv::namedWindow("Sinogram", cv::WINDOW_NORMAL);
  cv::imshow("Sinogram", compare);
  cv::waitKey(0);
  return 0;
}
