file(GLOB_RECURSE headers *.h)

if(ANKI_HEADLESS)
	set(sources InputDummy.cpp)
elseif(SDL)
	set(sources InputSdl.cpp)
elseif(ANDROID)
	set(sources InputAndroid.cpp)
endif()

add_library(AnKiInput ${sources} ${headers})
target_compile_definitions(AnKiInput PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiInput AnKiCore AnKiImGui)
