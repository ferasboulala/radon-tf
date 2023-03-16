#include <cmath>
#include <cstdint>
#include <thread>
#include <vector>

#include "tensor.hpp"

namespace {

struct PolarCoordinate {
    double radius;
    double theta;
};

using AngleCache = Tensor<PolarCoordinate, 2>;

template <typename T>
struct ThreadData {
    const T*                   src;
    T*                         dst;
    unsigned                   rows, cols;
    unsigned                   start_index, end_index, resolution, diagonal_length;
    double                     dtheta;
    const AngleCache*          angle_cache;
    const std::vector<double>* cosine_table;
};

std::vector<double> precompute_cosines(unsigned resolution) {
    assert(resolution);
    const double        dtheta = 2 * M_PI / resolution;
    std::vector<double> cosine_table(resolution);
    for (unsigned i = 0; i < cosine_table.size(); ++i) {
        cosine_table[i] = std::cos(i * dtheta);
    }

    return cosine_table;
}

template <typename T, unsigned N>
inline T l2_norm(const std::array<T, N>& size) {
    T accumulator = T(0);
    for (unsigned i = 0; i < N; ++i) {
        accumulator += size[i] * size[i];
    }

    return std::pow(accumulator, double(1) / N);
}

AngleCache precompute_angle_cache(unsigned cols, unsigned rows) {
    const double origin_x = std::round(cols / 2.0);
    const double origin_y = std::round(rows / 2.0);
    AngleCache   angle_cache({cols, rows});
    for (unsigned x = 0; x < angle_cache.size()[0]; ++x) {
        for (unsigned y = 0; y < angle_cache.size()[1]; ++y) {
            const double delta_x = x - origin_x;
            const double delta_y = origin_y - y;
            const double radius = l2_norm<double, 2>({delta_x, delta_y});
            const double theta = atan2(delta_y, delta_x);
            angle_cache[{x, y}] = {radius, theta};
        }
    }

    return angle_cache;
}

inline double normalize_angle(double angle) {
    angle = std::fmod(angle, 2 * M_PI);
    if (angle < M_PI) {
        angle += 2 * M_PI;
    }

    return angle;
}

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnarrowing"
#pragma GCC diagnostic ignored "-Wsign-compare"

template <typename T>
void transform_threaded(ThreadData<T>& thread_data) {
    const double diagonal_middle = thread_data.diagonal_length / 2;
    for (int y = 0; y < thread_data.rows; ++y) {
        for (int x = 0; x < thread_data.cols; ++x) {
            const auto polar_coordinate = thread_data.angle_cache->operator[]({x, y});
            const double theta = normalize_angle(polar_coordinate.theta -
                                                 thread_data.start_index * thread_data.dtheta);
            int          theta_index = (theta / (2 * M_PI)) * thread_data.cosine_table->size();
            const T      intensity =
                thread_data.src[ravel<2>({y, x}, {thread_data.rows, thread_data.cols})];
            for (int i = thread_data.start_index; i < thread_data.end_index; ++i, --theta_index) {
                const double                                            x_cart =
                    polar_coordinate.radius * thread_data.cosine_table->operator[](
                                                  (theta_index + thread_data.cosine_table->size()) %
                                                  thread_data.cosine_table->size());
                const int bin_idx = x_cart + diagonal_middle;
                thread_data.dst[ravel<2>({i, bin_idx},
                                         {thread_data.resolution, thread_data.diagonal_length})] +=
                    intensity;
            }
        }
    }
}

#pragma GCC diagnostic pop
}  // namespace

inline std::tuple<unsigned, unsigned> radon::sinogram_size(unsigned rows,
                                                           unsigned cols,
                                                           unsigned resolution) {
    const unsigned diagonal_length = l2_norm<unsigned, 2>({cols, rows});

    return {resolution, diagonal_length};
}

template <typename InputType, typename OutputType = float>
std::unique_ptr<OutputType[]> radon::transform(
    const InputType* src, unsigned rows, unsigned cols, unsigned resolution, unsigned n_threads) {
    const unsigned                diagonal_length = l2_norm<unsigned, 2>({cols, rows});
    const uint64_t                n_pixels_in_output = diagonal_length * resolution;
    const uint64_t                n_bytes_in_output = n_pixels_in_output * sizeof(OutputType);
    std::unique_ptr<OutputType[]> output(new OutputType[n_pixels_in_output]);
    std::memset(output.get(), 0, n_bytes_in_output);

    constexpr bool    output_same_as_input_type = std::is_same<InputType, OutputType>::value;
    const OutputType* copy;
    if constexpr (!output_same_as_input_type) {
        const uint64_t n_pixels_in_input = rows * cols;
        auto           new_src = new OutputType[n_pixels_in_input];
        for (uint64_t i = 0; i < n_pixels_in_input; ++i) {
            new_src[i] = src[i];
        }
        copy = new_src;
    } else {
        copy = src;
    }

    const std::vector<double> cosine_table = precompute_cosines(2 * resolution);
    const AngleCache          angle_cache = precompute_angle_cache(cols, rows);

    const double                         dtheta = M_PI / resolution;
    const int                            bin_per_thread = resolution / n_threads;
    std::vector<std::thread>             threads(n_threads);
    std::vector<ThreadData<OutputType> > data;
    data.reserve(n_threads);
    for (unsigned i = 0; i < n_threads; i++) {
        data.push_back(ThreadData<OutputType>{
            .src = copy,
            .dst = output.get(),
            .rows = rows,
            .cols = cols,
            .start_index = i * bin_per_thread,
            .end_index = (i == (n_threads - 1)) ? resolution : ((i + 1) * bin_per_thread),
            .resolution = resolution,
            .diagonal_length = diagonal_length,
            .dtheta = dtheta,
            .angle_cache = &angle_cache,
            .cosine_table = &cosine_table});
        threads[i] = std::thread(transform_threaded<OutputType>, std::ref(data[i]));
    }

    for (auto& t : threads) {
        t.join();
    }

    if constexpr (!output_same_as_input_type) {
        delete[] copy;
    }

    return output;
}