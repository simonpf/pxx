import numpy as np
import eigen_tensor

def test_add():

    a = np.random.rand(10, 20, 30)
    b = np.random.rand(10, 20, 30)

    c1 = eigen_tensor.add(a, b)
    c2 = a + b
    assert(all(np.isclose(c1, c2)))
