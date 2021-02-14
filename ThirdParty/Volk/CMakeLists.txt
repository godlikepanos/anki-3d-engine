file(GLOB_RECURSE SOURCE_FILES *.c)
include_directories(../Khronos)
add_compile_definitions("VK_KHR_ray_tracing_pipeline" "VK_KHR_acceleration_structure")
add_library(ankivolk ${SOURCE_FILES})
