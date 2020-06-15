configure_file(test_vector.py test_vector.py)

include_directories("${PROJECT_SOURCE_DIR}/ext/")
include_directories("${PROJECT_SOURCE_DIR}/include/")
include_directories(${Python_INCLUDE_DIRS})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

find_package(Eigen3)
if (TARGET Eigen3::Eigen)
  add_pxx_module(SOURCE eigen_vector.h MODULE eigen_vector INCLUDE_DIRS /home/simon/src/eigen)
  target_link_libraries(eigen_vector Eigen3::Eigen)
endif (TARGET Eigen3::Eigen)
