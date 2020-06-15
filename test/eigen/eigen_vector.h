#include "Eigen/Core"
#include <iostream>

using Matrix = Eigen::MatrixXd;
using Vector = Eigen::VectorXd;

// pxx :: export
Matrix add(Matrix a, Matrix b) {
    return a + b;
}

// pxx :: export
Matrix matrix() {
    Matrix matrix(2, 2);
    matrix.fill(1.0);
    return matrix;
}

// pxx :: export
class A {
public:
    void matrix () {
    }
};
