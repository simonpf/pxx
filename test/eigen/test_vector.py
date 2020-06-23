import vector

def test_add():
    m = np.random.randint(5, 20)
    n = np.random.randint(5, 20)

    a = np.random.rand(m, n)
    b = np.random.rand(m, n)

    c1 = vector.add(a, b)
    c2 = a + b
    assert(all(np.isclose(c1, c2)))
