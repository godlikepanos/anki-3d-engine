file(GLOB_RECURSE sources *.cpp)
add_library(AnKiPhysics ${sources})
target_compile_definitions(AnKiPhysics PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiPhysics AnKiUtil BulletSoftBody BulletDynamics BulletCollision LinearMath)
