find_package(Catch2 CONFIG REQUIRED)

set(TARGET_NAME "${PROJECT_NAME}-test")
add_executable("${TARGET_NAME}" "main.cpp" "bonicpp/test_mdvector.cpp")

target_link_libraries("${TARGET_NAME}" PRIVATE "${PROJECT_NAME}-common")
target_link_libraries("${TARGET_NAME}" PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests("${TARGET_NAME}")
