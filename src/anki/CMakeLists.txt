set(ANKI_SUB_DIRS importer core script renderer scene ui input physics resource gr collision math util shader_compiler)
foreach(TMP ${ANKI_SUB_DIRS})
	add_subdirectory(${TMP})
endforeach()

set(AK_SOURCES ${AK_SOURCES} PARENT_SCOPE)
