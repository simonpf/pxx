import numpy as np
import class_template as ct

def test_class_templates():

    s = ct.Sum()

    assert(len(s.public_data) == 3)
    assert(np.all(np.isclose(s.public_data, np.ones(3))))
