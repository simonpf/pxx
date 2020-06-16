configure_file(test_vector.py test_vector.py)
add_pxx_module(
  SOURCE vector.h
  MODULE standard_vector
  DEPENDS ${pxx_source_files} _pxx pxx_python
  )
