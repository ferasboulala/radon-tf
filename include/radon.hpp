#include <opencv2/opencv.hpp>
#include <math.h>
#include <vector>
#include <thread>

typedef std::pair<float, float> Coord;
typedef std::vector<std::vector<Coord>> Map;

struct thread_data {
  cv::Mat dst, src;
  int start, end;
  float d_theta, p_mid;
  Map* map;
};

namespace cv {
  void sinogram_p(struct thread_data thread);
  cv::Mat sinogram(const cv::Mat& src, const int theta_bin, const int n_threads = 1);
  void reconstruct_p(struct thread_data thread);
  cv::Mat reconstruct(const cv::Mat& src, const cv::Size size, const int n_threads = 1);
  void radon_normalize(cv::Mat& src, const int type, bool sc = true);
}
