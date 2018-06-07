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
  img.convertTo(img, CV_32F);
  cv::resize(img, img, cv::Size(256,256));
  cv::Mat rad = Radon::sinogram(img, 256);
  
  double min, max;
  cv::minMaxLoc(rad, &min, &max);
  rad = rad / max;

  cv::namedWindow("Sinogram", cv::WINDOW_NORMAL);
  cv::imshow("Sinogram", rad);
  cv::waitKey(0);
  return 0;
}
