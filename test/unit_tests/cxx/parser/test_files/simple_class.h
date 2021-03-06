/** simple_class.h
 *
 * This is a simple test file that defines a class to test the
 * parsing of the class and its child nodes.
 *
 */
class A {

public:

  A() {}
  A(int) {}

  /// A public method.
  void public_method_1() {}
  /// Another public method.
  int public_method_1(int) {}

  /** A doxygen documented method.
   * @param a A parameter.
   */
  int public_method_2(float) { return 0 }

  char public_member = 'a';

private:
  int private_member = 0;
  int private_method(int) {}

protected:
  int protected_member = 0;
  int protected_method(int) {}
};
