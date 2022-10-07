#include "catch2/catch_test_macros.hpp"
#include "pxx/cxx/type_names.h"

TEST_CASE( "is_qualified", "[cxx/type_name]" ) {

    using pxx::cxx::type_names::is_qualified;

    std::string qualified_name = "std::string";
    REQUIRE(is_qualified(qualified_name));
}

