configure_file(test_function_template.py test_function_template.py)
add_pxx_module(
  SOURCE function_template.h
  MODULE function_template
  DEPENDS ${pxx_source_files} _pxx pxx_python
  )

configure_file(test_class_template.py test_class_template.py)
add_pxx_module(
  SOURCE class_template.h
  MODULE class_template
  DEPENDS ${pxx_source_files} _pxx pxx_python
  )
