add_executable(physics_playground Main.cpp ../common/Framework.cpp)
target_link_libraries(physics_playground anki)

installExecutable(physics_playground)
