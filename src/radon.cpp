#include "radon.hpp"

cv::Mat cv::sinogram(const cv::Mat& src, const int theta_bin){
  const int p_bin = round(sqrt(pow(src.rows, 2) + pow(src.cols, 2)));
  cv::Mat dst(theta_bin, p_bin, CV_64F, cv::Scalar(0));
  cv::Mat copy;
  src.copyTo(copy);
  copy.convertTo(copy, CV_64F);

  Map map;
  map.resize(src.cols); // x
  for (auto it = map.begin(); it != map.end(); it++)
    it->resize(src.rows); // y

  const Coord origin = {floor(src.cols/2.0), floor(src.rows/2.0)};
  for (int x = 0; x < map.size(); x++){
    for (int y = 0; y < map[x].size(); y++){
      Coord cartesian = {x - origin.first, origin.second - y};
      float r = sqrt(pow(cartesian.first, 2) + pow(cartesian.second, 2));
      float theta;
      theta = atan2(cartesian.second, cartesian.first);
      Coord polar = {r, theta};
      map[x][y] = polar;
    }
  }

  const float p_mid = p_bin / 2;
  const float d_theta = M_PI / theta_bin;
  for (int i = 0; i < theta_bin; i++){
    for (int x = 0; x < map.size(); x++){
      for (int y = 0; y < map[x].size(); y++){
        float x_cart = map[x][y].first * cos(map[x][y].second);
        int bin_idx = round(x_cart + p_mid);
        dst.at<double>(i,bin_idx) += copy.at<double>(y,x);
        map[x][y].second -= d_theta;
      }
    }
  }

  double min, max;
  cv::minMaxLoc(dst, &min, &max);
  dst = dst/max;
  double scale = 1;
  bool offset = false;
  switch(src.type()){
    case CV_8U : {
      scale = 255;
      break;
    }
    case CV_8S : {
      scale = 127;
      offset = true;
      break;
    }
    case CV_16U : {
      scale = pow(256, 2) - 1;
      break;
    }
    case CV_16S : {
      scale = pow(256, 2) / 2 - 1;
      offset = true;
      break;
    }
    case CV_32S : {
      scale = pow(256, 4) / 2 - 1;
      offset = true;
      break;
    }
    case CV_USRTYPE1 : {
      std::cout << "radon-tf does't support user type scaling." << std::endl;
      break;
    }
  }
  dst = dst * scale - offset;
  if (offset)
    dst = dst - offset;
  dst.convertTo(dst, src.type());
  return dst;
}

cv::Mat cv::sinogram_p(const cv::Mat& src, const int theta_bin){

}

int cv::backprojection(const cv::Mat src, std::vector<cv::Mat> dst, const uint res){

}

int cv::reconstruct(const std::vector<cv::Mat> src, cv::Mat dst){

}
