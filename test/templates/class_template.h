#include <array>

#define ARRAY std::array<T, N>

template<typename T>
struct Hidden {
    using type = T;
};

namespace detail {
    class A {};
    class B {
    public:
        using Alias = A;
    };
}

// pxx :: export
// pxx :: instance(["int", "3"])
template <typename T, int N>
class Sum {

public:
    using ReturnType = decltype(static_cast<T>(1.0));
    using A = typename detail::B::Alias;

  Sum() {
      data.fill({0, 0, 0});
      public_data.fill({1, 1, 1});
  }
  std::array<std::array<ReturnType, N>, N> get_data(int a) {return data; }
  std::array<std::array<T, N>, N> get_data() { return data; }

  int get(int a) { return a; }
  A get() { return A{}; }

  std::array<std::array<ReturnType, N>, N> public_data;


 private:
  std::array<std::array<ReturnType, N>, N> data;
};
