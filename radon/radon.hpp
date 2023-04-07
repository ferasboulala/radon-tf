#include <memory>

namespace radon {

// Given an input image size and a theta resolution, returns <number of rows, number of columns> in
// the expected sinogram.
inline std::tuple<unsigned, unsigned> sinogram_size(unsigned rows,
                                                    unsigned cols,
                                                    unsigned resolution);

// Returns a sinogram. Use appropriate data types to avoid overflow.
template <typename InputType, typename OutputType = float>
std::unique_ptr<OutputType[]> transform(const InputType* src,
                                        unsigned         rows,
                                        unsigned         cols,
                                        unsigned         resolution,
                                        unsigned         n_threads = 1);

}  // namespace radon

#include "radon.inl.hpp"
