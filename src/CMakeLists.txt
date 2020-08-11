include_directories("${PROJECT_SOURCE_DIR}/ext/")
include_directories("${PROJECT_SOURCE_DIR}/include/")

find_package(LibClang)
find_package(Python COMPONENTS Development Interpreter)

include_directories(${Python_INCLUDE_DIRS})
add_library(_pxx SHARED pxx.cxx)
set_target_properties(_pxx PROPERTIES PREFIX "")
target_link_libraries(_pxx ${LIBCLANG_LIBRARIES})
