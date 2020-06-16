configure_file(test_vector.py test_vector.py)

find_package(Eigen3)
if (TARGET Eigen3::Eigen)
  add_pxx_module(
    SOURCE eigen_vector.h
    MODULE eigen_vector INCLUDE_DIRS /home/simon/src/eigen
    DEPENDS ${pxx_source_files} _pxx pxx_python
    )
  target_link_libraries(eigen_vector Eigen3::Eigen)
endif (TARGET Eigen3::Eigen)
