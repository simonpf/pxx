
find_package(Eigen3)
get_target_property(Eigen3_INCLUDE_DIRS Eigen3::Eigen INTERFACE_INCLUDE_DIRECTORIES)
set(eigen_unsupported_include_dir "${Eigen3_INCLUDE_DIRS}/unsupported")
message(WARNING ${Eigen3_INCLUDE_DIRS})
message(WARNING ${eigen_unsupported_include_dir})

if (TARGET Eigen3::Eigen)
  configure_file(test_vector.py test_vector.py)
  add_pxx_module(
    SOURCE eigen_vector.h
    MODULE eigen_vector
    INCLUDE_DIRS ${Eigen3_INCLUDE_DIRS} ${eigen_unsupported_include_dir}
    DEPENDS ${pxx_source_files} _pxx pxx_python
    )

  configure_file(test_tensor.py test_tensor.py)
  add_pxx_module(
    SOURCE tensor.h
    MODULE eigen_tensor
    INCLUDE_DIRS ${Eigen3_INCLUDE_DIRS} ${eigen_unsupported_include_dir}
    DEPENDS ${pxx_source_files} _pxx pxx_python
    )
endif (TARGET Eigen3::Eigen)
