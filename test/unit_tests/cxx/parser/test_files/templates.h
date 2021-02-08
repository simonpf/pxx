/** template.h
 *
 * A test file that defines different templates to test the parsing
 * of templates.
 *
 */
template<typename T>
void function(T t) {}


template<typename t, int N>
class Class {

};


template <typename t>
class Class<t, 0> {

};


template class Class<int, 0>;
