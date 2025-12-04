find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}-common")
add_library("${TARGET_NAME}" "argparse.cpp" "logging.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)
target_link_libraries("${TARGET_NAME}" PRIVATE spdlog::spdlog)

find_package(argparse CONFIG REQUIRED)
find_package(spdlog CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}")
add_executable("${TARGET_NAME}" "main.cpp")
set_target_properties(
  "${TARGET_NAME}" PROPERTIES CXX_STANDARD 20 CXX_EXTENSIONS OFF
)

target_link_libraries("${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-common")
target_link_libraries("${TARGET_NAME}" PRIVATE argparse::argparse)

if(BONICPP_BUILD_RAYTRACING)
  add_subdirectory("raytracing")
endif()
