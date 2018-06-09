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

  radon_normalize(dst, src.type());
  return dst;
}

void cv::reconstruct_p(struct thread_data thread){
  for (int i = thread.start; i < thread.end; i++){
    for (int y = 0; y < (*thread.map).size(); y++){
      for (int x = 0; x < (*thread.map)[0].size(); x++){
        float theta = (*thread.map)[x][y].second;
        float r = (*thread.map)[x][y].first;
        theta += thread.d_theta * i;
        Coord cartesian = {round(r * cos(theta) + thread.p_mid), round(-r * sin(theta) + thread.p_mid)};
        if (cartesian.first > thread.src.cols - 1 || cartesian.second > thread.src.cols - 1
            || cartesian.second < 0 || cartesian.first < 0)
          continue;
        thread.dst.at<double>(cartesian.second,cartesian.first) += thread.src.at<double>(i,x);
      }
    }
  }
}

cv::Mat cv::reconstruct(const cv::Mat& src, const cv::Size size, const int n_threads){
  const int p_bin = src.cols;
  const int theta_bin = src.rows;
  cv::Mat dst(p_bin, p_bin, CV_64F, cv::Scalar(0));
  cv::Mat copy;
  src.copyTo(copy);
  copy.convertTo(copy, CV_64F);

  Map map;
  map.resize(p_bin);
  for (int i = 0; i < map.size(); i++){
    map[i].resize(p_bin);
  }

  const Coord origin = {round(p_bin/2.0), round(p_bin/2.0)};
  for (int x = 0; x < map.size(); x++){
    for (int y = 0; y < map[x].size(); y++){
      Coord cartesian = {x - origin.first, origin.second - y};
      float r = sqrt(pow(cartesian.first, 2) + pow(cartesian.second, 2));
      float theta = atan2(cartesian.second, cartesian.first);
      Coord polar = {r, theta};
      map[x][y] = polar;
    }
  }

  const float p_mid = p_bin / 2.0;
  const float d_theta = M_PI / theta_bin;
  std::vector<std::thread> threads(n_threads);
  std::vector<struct thread_data> data(n_threads);
  const int bin_per_thread = theta_bin / n_threads;
  for (int i = 0; i < n_threads; i++){
    const int start = i * bin_per_thread;
    int end = (i + 1) * bin_per_thread;
    if (i == n_threads - 1)
      end = theta_bin;
    // Creating an image for each thread to avoid mutex locks
    cv::Mat thread_img, thread_copy;
    dst.copyTo(thread_img);
    copy.copyTo(thread_copy);
    data[i].dst     = thread_img;
    data[i].src     = thread_copy;
    data[i].start   = start;
    data[i].end     = end;
    data[i].d_theta = d_theta;
    data[i].p_mid   = p_mid;
    data[i].map     = &map;
    threads[i] = std::thread(cv::reconstruct_p, std::ref(data[i]));
  }
  int i = 0;
  for (auto it = threads.begin(); it != threads.end(); it++){
    it->join();
    dst += data[i].dst;
    i++;
  }

  dst = dst(Rect(p_mid - size.width/2, p_mid - size.height/2, size.width, size.height));
  radon_normalize(dst, src.type());
  return dst;
}

void cv::radon_normalize(cv::Mat& src, const int type, bool sc){
  double min, max;
  cv::minMaxLoc(src, &min, &max);
  if (sc)
    src = src / max;
  double scale = 1;
  bool offset = false;
  switch(type){
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
  src = src * scale - offset;
  if (offset)
    src = src - offset;
  src.convertTo(src, type);
}
