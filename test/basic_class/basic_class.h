

// pxx :: export
class TestClass {
public:
  TestClass(){};
  TestClass(int a) : a_(a){};

  const char* get_string() {return "hello";}
  int get_int() {return 42;}

  int public_member_1 = 1;
  const int public_member_2 = 2;

private:
  int a_ = 99;
  int b_ = 99;
};
