set(ANKI_SUB_DIRS Importer Core Script Renderer Scene Ui Window Resource Gr Collision Math Util ShaderCompiler Shaders Physics GpuMemory)

if(ANKI_WITH_EDITOR)
	set(ANKI_SUB_DIRS ${ANKI_SUB_DIRS} Editor)
endif()

foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(${TMP})
endforeach()

set(AK_SOURCES ${AK_SOURCES} PARENT_SCOPE)
