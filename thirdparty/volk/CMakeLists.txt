file(GLOB_RECURSE SOURCE_FILES *.c)
include_directories(../khronos)
add_compile_definitions("VK_KHR_ray_tracing" "VK_ENABLE_BETA_EXTENSIONS")
add_library(ankivolk ${SOURCE_FILES})
