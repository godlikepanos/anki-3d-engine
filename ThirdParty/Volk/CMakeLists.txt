file(GLOB_RECURSE SOURCE_FILES *.c)
include_directories(../Khronos)
add_definitions("-DVK_KHR_ray_tracing_pipeline" "-DVK_KHR_acceleration_structure")
add_library(AnKiVolk ${SOURCE_FILES})
