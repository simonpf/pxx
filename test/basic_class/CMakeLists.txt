configure_file(test_class.py test_class.py)
add_pxx_module(
  SOURCE basic_class.h
  MODULE basic_class
  DEPENDS ${pxx_source_files} _pxx pxx_python
  )
