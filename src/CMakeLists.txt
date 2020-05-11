include_directories("${PROJECT_SOURCE_DIR}/ext/")
include_directories("${PROJECT_SOURCE_DIR}/include/")

find_package(Python COMPONENTS Development Interpreter)
message(WARNING "Python: ${Python_EXECUTABLE}")

include_directories(${Python_INCLUDE_DIRS})

find_package(LibClang)

add_library(_pxx SHARED pxx.cxx)
set_target_properties(_pxx PROPERTIES PREFIX "")
target_link_libraries(_pxx ${LIBCLANG_LIBRARIES})
