#include <array>

#define ARRAY std::array<T, N>

// pxx :: export
// pxx :: instance(["int", "3"])
template <typename T, int N>
class Sum {
public:
  Sum() {
      data.fill(0);
      public_data.fill(1);
  }
  ARRAY get_data(int a) const { return data; }

  int get(int a) { return a; }

  std::array<T, N> public_data;

  template<typename T>
      void foo(const T &t) {
  }

 private:
  std::array<T, N> data;
};
