#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <opencv2/opencv.hpp>
#include <string>

#include "radon/radon.hpp"

static constexpr unsigned RESOLUTION = 720;
using RadonType = double;

template <typename T>
static T cv_max(const cv::Mat img) {
    T        val = std::numeric_limits<T>::min();
    const T *data = (T *)img.data;
    for (size_t i = 0; i < img.total(); ++i) {
        if (data[i] > val) val = data[i];
    }

    return val;
}

void print_help(char *progname) {
    printf("Usage: %s [-t num_threads] input_file output_file\n", progname);
    printf(
        "This program reads an input image in grey scale and evaluates the radon transform. The "
        "output is stored to disk.\n");
    printf("Options:\n");
    printf("  -t num_threads  Number of threads to use (default: 1)\n");
    printf("  -h, --help      Display this help message\n");
}

int main(int argc, char **argv) {
    static constexpr uint64_t DEFAULT_N_THREADS = 1;

    int opt;
    int num_threads = DEFAULT_N_THREADS;

    while ((opt = getopt(argc, argv, "ht:")) != -1) {
        switch (opt) {
            case 't':
                num_threads = atoi(optarg);
                break;
            case 'h':
            case '?':
                print_help(argv[0]);
                return 0;
        }
    }

    if (optind + 2 > argc) {
        fprintf(stderr, "Usage: %s [-t num_threads] input_file output_file\n", argv[0]);
        return 1;
    }

    const char *input_file = argv[optind];
    const char *output_file = argv[optind + 1];

    printf("Input file: %s\n", input_file);
    printf("Output file: %s\n", output_file);
    printf("Number of threads: %d\n", num_threads);

    cv::Mat img = cv::imread(input_file, cv::IMREAD_GRAYSCALE);

    std::chrono::high_resolution_clock::time_point start, end;
    std::chrono::duration<double, std::milli>      fp_ms;
    start = std::chrono::high_resolution_clock::now();
    auto sinogram =
        radon::transform<uint8_t, RadonType>(img.data, img.rows, img.cols, RESOLUTION, num_threads);
    end = std::chrono::high_resolution_clock::now();
    fp_ms = end - start;
    printf("Took %f ms\n", fp_ms.count());

    const auto [n_rows, n_cols] = radon::sinogram_size(img.rows, img.cols, RESOLUTION);
    const auto data = sinogram.get();
    sinogram.release();
    cv::Mat cv_sinogram(n_rows, n_cols, CV_64FC1, data);

    const RadonType largest_intensity = cv_max<RadonType>(cv_sinogram);
    cv_sinogram *= 255 / largest_intensity;
    cv_sinogram.convertTo(cv_sinogram, CV_8UC1);

    cv::imwrite(output_file, cv_sinogram);

    return 0;
}
