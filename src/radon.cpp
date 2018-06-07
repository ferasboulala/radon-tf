#include "radon.hpp"

cv::Mat Radon::sinogram(const cv::Mat& src, const int theta_bin){
  const int p_bin = sqrt(pow(src.rows, 2) + pow(src.cols, 2));
  cv::Mat dst(theta_bin, p_bin, src.type(), cv::Scalar(0));

  Map map;
  map.resize(src.cols); // x
  for (auto it = map.begin(); it != map.end(); it++)
    it->resize(src.rows); // y

  const Coord origin = {src.cols/2.0, src.rows/2.0};
  for (int x = 0; x < map.size(); x++){
    for (int y = 0; y < map[x].size(); y++){
      // Cartesian to polar conversion
      Coord cartesian = {x - origin.first, origin.second - y};
      float r = sqrt(pow(cartesian.first, 2) + pow(cartesian.second, 2));
      float theta = 0.0;
      if (cartesian.first)
        theta = atan2(cartesian.second, cartesian.first);
      Coord polar = {r, theta};
      map[x][y] = polar;
    }
  }

  const int p_mid = p_bin/2;
  const float d_theta = M_PI / theta_bin / 2;
  for (int i = 0; i < theta_bin; i++){
    for (int x = 0; x < map.size(); x++){
      for (int y = 0; y < map[x].size(); y++){
        float x_cart = map[x][y].first * cos(map[x][y].second);
        int bin_idx = floor(x_cart + p_mid);
        dst.at<float>(i, bin_idx) += src.at<float>(y,x);
        map[x][y].second -= d_theta;
      }
    }
  }
  return dst;
}

int Radon::backprojection(const cv::Mat src, std::vector<cv::Mat> dst, const uint res){

}

int Radon::reconstruct(const std::vector<cv::Mat> src, cv::Mat dst){

}
