message("Going to download Googletest")
include(FetchContent)
FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG release-1.12.1
)

# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)
message("Googletest downloaded")

enable_testing()

add_executable(
    tests
    test_correlation.cpp
    test_helpers.cpp
    ../src/benchmarks/analysis/correlation.cpp
    ../src/helpers/helper_functions.cpp
    ../src/benchmarks/linux/struct_sensors.cpp
     ../src/benchmarks/linux/synchronizer.cpp
)
target_compile_features(tests PRIVATE cxx_std_17)
target_link_libraries(
    tests
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(tests)
