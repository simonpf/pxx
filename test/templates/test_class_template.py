import numpy as np
import class_template as ct

def test_class_template():
    s = ct.Sum()
    assert(len(s.public_data) == 3)
    assert(np.all(np.isclose(s.public_data, np.ones((3, 3)))))
    assert(np.all(np.isclose(s.get_data(), np.zeros((3, 3)))))

def test_class_template_partial_specialization():
    s = ct.Sum1()
    assert(type(s.public_data) == int)
    assert(type(s.get_data()) == int)
