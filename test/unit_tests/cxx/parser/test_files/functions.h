/** \file test/unit_tests/cxx/parser/test_files/functions.h
 *
 * This file contains defintions of functions to test the parsing of
 * functions.
 */
int function1() { return 1; }

int function1(float, double) { return 1; }

namespace my_namespace {
int function2(std::string &s, MyType t) { return 2; }
} // namespace my_namespace
