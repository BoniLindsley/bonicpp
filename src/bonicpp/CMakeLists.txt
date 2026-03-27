find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}-common")
add_library("${TARGET_NAME}" "argparse.cpp" "logging.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)
target_include_directories(
  "${TARGET_NAME}" INTERFACE "${CMAKE_SOURCE_DIR}/src"
)
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)
target_link_libraries("${TARGET_NAME}" PRIVATE spdlog::spdlog)

if(BONICPP_BUILD_RAYTRACING)
  add_subdirectory("raytracing")
endif()
# add_subdirectory("watchdog")
