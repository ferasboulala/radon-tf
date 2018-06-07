#include <opencv2/opencv.hpp>
#include <math.h>
#include <vector>

typedef std::pair<float, float> Coord;
typedef std::vector<std::vector<Coord>> Map;

namespace cv{
  cv::Mat sinogram(const cv::Mat& src, const int theta_bin);
  int backprojection(const cv::Mat src, std::vector<cv::Mat> dst, const uint res);
  int reconstruct(const std::vector<cv::Mat> src, cv::Mat dst);
  cv::Mat sinogram_p(const cv::Mat& src, const int theta_bin);
}
