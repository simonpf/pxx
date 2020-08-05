/** Test function template wrapper.
 *
 * Input file with different function templates to test the wrapping of
 * function templates.
 */
#include <iostream>
#include <array>

// pxx :: export
// pxx :: instance(["float", "3"])
// pxx :: instance(["double", "4"])
template<typename Scalar, size_t N>
Scalar sum(std::array<Scalar, N> x) {
    Scalar result = 0.0;
    for (size_t i = 0; i < N; ++i) {
        result += x[i];
    }
    return result;
}

namespace detail {

    // pxx :: instance("hidden_sum", ["float", "3"])
    // pxx :: export
    // pxx :: instance("hidden_sum", ["double", "4"])
    template<typename Scalar, size_t N>
    Scalar sum(std::array<Scalar, N> x) {
    Scalar result = 0.0;
    for (size_t i = 0; i < N; ++i) {
        result += x[i];
    }
    return result;
    }

}

// pxx :: export
void test(int /*a*/) {
    // nada.
}

