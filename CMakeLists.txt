cmake_minimum_required(VERSION 3.22)
project("bonicpp" VERSION 0.1.0)

add_subdirectory("src")

option(BONICPP_BUILD_TESTING "Build tests" OFF)
if(BONICPP_BUILD_TESTING)
  enable_testing()
  add_subdirectory("tests")
endif()

find_package(Doxygen)
doxygen_add_docs(doc)

include(CPack)
