#include <array>

#define ARRAY std::array<T, N>

template<typename T>
struct Hidden {
    using type = T;
};

// pxx :: export
// pxx :: instance(["int", "3"])
template <typename T, int N>
class Sum {

    using ReturnType = decltype(static_cast<T>(1.0));

public:
  Sum() {
      data.fill(0);
      public_data.fill(1);
  }
  std::array<T, N> get_data(int a) {return data; }
  std::array<T, N> get_data() { return data; }

  int get(int a) { return a; }

  std::array<ReturnType, N> public_data;


 private:
  std::array<T, N> data;
};
