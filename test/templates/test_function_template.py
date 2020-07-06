import numpy as np
import function_template as ft

def test_function_templates():
    size = 3
    l = np.random.randint(0, 20, size)
    sum1 = ft.sum(l)
    sum2 = ft.hidden_sum(l)
    assert(sum1 == sum2)

    size = 4
    l = np.random.randint(0, 20, size)
    sum1 = ft.sum(l)
    sum2 = ft.hidden_sum(l)
    assert(sum1 == sum2)

