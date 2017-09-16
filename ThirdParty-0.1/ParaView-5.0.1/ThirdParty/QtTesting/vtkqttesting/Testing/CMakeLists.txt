if(NOT ${CMAKE_Cxx_Fortran_COMPILER_ID} STREQUAL "Intel")
  # QtTest fails to compile on ICC. Hence we don't add these tests on
  # Intel compilers.
  add_subdirectory(Cpp)
endif()

