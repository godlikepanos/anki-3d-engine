include_directories(${CMAKE_CURRENT_SOURCE_DIR})
add_definitions("-DHWCPIPE_NO_JSON")
add_library(AnKiHwcPipe "hwcpipe.cpp" "vendor/arm/mali/mali_profiler.cpp" "vendor/arm/pmu/pmu_counter.cpp" "vendor/arm/pmu/pmu_profiler.cpp")

