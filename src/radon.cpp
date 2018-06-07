#include "radon.hpp"

void cv::sinogram_p(struct thread_data thread){
  for (int x = 0; x < thread.map->size(); x++){
    for (int y = 0; y < (*thread.map)[0].size(); y++){
      float theta = (*thread.map)[x][y].second - thread.start * thread.d_theta;
      float r = (*thread.map)[x][y].first;
      for (int i = thread.start; i < thread.end; i++){
        float x_cart = r * cos(theta);
        int bin_idx = round(x_cart + thread.p_mid);
        thread.dst.at<double>(i,bin_idx) += thread.src.at<double>(y,x);
        theta -= thread.d_theta;
      }
    }
  }
}

cv::Mat cv::sinogram(const cv::Mat& src, const int theta_bin, const int n_threads){
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

  // Threading
  const float p_mid = p_bin / 2;
  const float d_theta = M_PI / theta_bin;
  std::vector<std::thread> threads(n_threads);
  std::vector<struct thread_data> data(n_threads);
  const int bin_per_thread = theta_bin / n_threads;
  for (int i = 0; i < n_threads; i++){
    const int start = i * bin_per_thread;
    int end = (i + 1) * bin_per_thread;
    if (i == n_threads - 1)
      end = theta_bin;
    data[i].dst     = dst;
    data[i].src     = copy;
    data[i].start   = start;
    data[i].end     = end;
    data[i].d_theta = d_theta;
    data[i].p_mid   = p_mid;
    data[i].map     = &map;
    threads[i] = std::thread(cv::sinogram_p, std::ref(data[i]));
  }
  for (auto it = threads.begin(); it != threads.end(); it++){
    it->join();
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

int cv::backprojection(const cv::Mat src, std::vector<cv::Mat> dst, const uint res){

}

int cv::reconstruct(const std::vector<cv::Mat> src, cv::Mat dst){

}
