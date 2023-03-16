#include <benchmark/benchmark.h>

#include <cassert>
#include <memory>
#include <opencv2/opencv.hpp>

#include "radon/radon.hpp"

static cv::Mat phantom_img;

template <typename InputType, typename OutputType>
void benchmark_phantom(benchmark::State& state) {
    std::unique_ptr<InputType[]> src(new InputType[phantom_img.total()]);
    for (uint64_t i = 0; i < phantom_img.total(); ++i) {
        src.get()[i] = phantom_img.at<uint8_t>(i);
    }

    static constexpr unsigned N_THETA_BINS = 720;
    for (auto _ : state) {
        auto sinogram = radon::transform<InputType, OutputType>(
            src.get(), phantom_img.rows, phantom_img.cols, N_THETA_BINS, 1);
        benchmark::DoNotOptimize(sinogram);
        benchmark::ClobberMemory();
    }
}

int main(int argc, char** argv) {
    assert(argc == 2);
    const std::string phantom_filename = argv[1];

    phantom_img = cv::imread(phantom_filename, cv::IMREAD_GRAYSCALE);
    // Some of these tests cause overflows but that is fine. We want to benchmark the execution, not
    // have valid data in the sinogram.
    std::vector<benchmark::internal::Benchmark*> benchmarks = {
        benchmark::RegisterBenchmark("u8", benchmark_phantom<uint8_t, uint8_t>),
        benchmark::RegisterBenchmark("u16", benchmark_phantom<uint16_t, uint16_t>),
        benchmark::RegisterBenchmark("u32", benchmark_phantom<uint32_t, uint32_t>),
        benchmark::RegisterBenchmark("u64", benchmark_phantom<uint64_t, uint64_t>),
        benchmark::RegisterBenchmark("f32", benchmark_phantom<float, float>),
        benchmark::RegisterBenchmark("f64", benchmark_phantom<double, double>),
        benchmark::RegisterBenchmark("u8u64", benchmark_phantom<uint8_t, uint64_t>),
        benchmark::RegisterBenchmark("u64f64", benchmark_phantom<uint64_t, double>),
        benchmark::RegisterBenchmark("f64u64", benchmark_phantom<double, uint64_t>),
    };

    static constexpr uint64_t N_ITERATIONS = 4;
    for (auto bench : benchmarks) {
        bench->Iterations(N_ITERATIONS);
    }

    int new_argc = 0;
    benchmark::Initialize(&new_argc, &argv[2]);
    benchmark::RunSpecifiedBenchmarks();
}