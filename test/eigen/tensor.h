#include "Eigen/Core"
#include "Eigen/CXX11/Tensor"
#include <iostream>

using Tensor3 = Eigen::Tensor<float, 3>;
using Tensor4 = Eigen::Tensor<float, 4, Eigen::RowMajor>;

// pxx :: export
Tensor3 add(Tensor3 a, Tensor3 b) {
    return a + b;
}
