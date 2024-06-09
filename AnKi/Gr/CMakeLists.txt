set(common_sources Common.cpp)

set(backend_sources
	Common.cpp
	GrObject.cpp
	RenderGraph.cpp
	ShaderProgram.cpp
	Utils/StackGpuMemoryPool.cpp
	BackendCommon/Functions.cpp
	BackendCommon/Common.cpp
	Utils/SegregatedListsGpuMemoryPool.cpp)

set(backend_headers
	AccelerationStructure.h
	Buffer.h
	CommandBuffer.h
	Common.h
	Fence.h
	GrManager.h
	GrObject.h
	OcclusionQuery.h
	RenderGraph.h
	RenderGraph.inl.h
	Sampler.h
	Shader.h
	ShaderProgram.h
	ShaderVariableDataType.def.h
	Texture.h
	TimestampQuery.h
	PipelineQuery.h
	GrUpscaler.h
	Utils/StackGpuMemoryPool.h
	BackendCommon/Functions.h
	BackendCommon/InstantiationMacros.def.h
	BackendCommon/Format.def.h
	Utils/SegregatedListsGpuMemoryPool.h)

if(VULKAN)
	file(GLOB_RECURSE vksources Vulkan/*.cpp)
	file(GLOB_RECURSE vkseaders Vulkan/*.h)

	set(backend_sources  ${backend_sources} ${vksources})
	set(backend_headers ${backend_headers} ${vkheaders})
elseif(DIRECTX)
	file(GLOB_RECURSE dxsources D3D/*.cpp)
	file(GLOB_RECURSE dxseaders D3D/*.h)

	set(backend_sources  ${backend_sources} ${dxsources})
	set(backend_headers ${backend_headers} ${dxheaders})

	find_library(PIX_LIB WinPixEventRuntime HINTS "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/Pix/bin/x64")

	set(extra_libs ${extra_libs} d3d12 dxgi dxguid ${PIX_LIB}) # dxguid for some debug layer stuff

	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/AgilitySdk/include")
	include_directories("${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/Pix/include/WinPixEventRuntime")
endif()

# Have 2 libraries. The AnKiGrCommon is the bare minimum for the AnKiShaderCompiler to work. Don't have
# AnKiShaderCompiler depend on what AnKiGr depends on

add_library(AnKiGrCommon ${common_sources})
target_compile_definitions(AnKiGrCommon PRIVATE -DANKI_SOURCE_FILE)
target_link_libraries(AnKiGrCommon AnKiUtil) # Only depend on Util

if(ANKI_DLSS)
	set(extra_libs ${extra_libs} AnKiNgx)
endif()

add_library(AnKiGr ${backend_sources} ${backend_headers})

set(COMPILE_DEFS "-DANKI_SOURCE_FILE")
if(DIRECTX)
	set(COMPILE_DEFS ${COMPILE_DEFS} "-DUSE_PIX")	
endif()
target_compile_definitions(AnKiGr PRIVATE ${COMPILE_DEFS})

target_link_libraries(AnKiGr AnKiCore AnKiSpirvCross AnKiGrCommon ${extra_libs})

# Copy DLSS files to Binaries
if(ANKI_DLSS)
	get_target_property(lib_type AnKiNgx TYPE)
	if("${lib_type}" STREQUAL "SHARED_LIBRARY")
		get_target_property(ANKINGX_DLL AnKiNgx IMPORTED_LOCATION)
		add_custom_command(
			TARGET AnKiGr
			COMMENT "Dependant DLLs copy ${ANKINGX_DLL} ${CMAKE_BINARY_DIR}/Binaries"
			COMMAND ${CMAKE_COMMAND} -E copy_if_different "${ANKINGX_DLL}" "${CMAKE_BINARY_DIR}/Binaries")
	else()
		get_property(__NGX_DLLS_LIST TARGET AnKiNgx PROPERTY ANKINGX_EXTRA_DLLS)
		foreach(dll_path ${__NGX_DLLS_LIST})
			file(GLOB dll_files "${dll_path}")
			foreach(dll_file ${dll_files})
				add_custom_command(
				TARGET AnKiGr
				COMMENT "NGX DLL copy ${dll_file} ${CMAKE_BINARY_DIR}/Binaries\n"
				COMMAND ${CMAKE_COMMAND} -E copy_if_different "${dll_file}" "${CMAKE_BINARY_DIR}/Binaries")
			endforeach()
		endforeach()
	endif()
endif()

# Copy Agility SDK files to Binaries
if(DIRECTX)
	file(GLOB FILES_TO_COPY "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/AgilitySdk/bin/x64/*.dll")
	file(GLOB FILES_TO_COPY2 "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/AgilitySdk/bin/x64/*.pdb")
	file(GLOB FILES_TO_COPY3 "${CMAKE_CURRENT_SOURCE_DIR}/../../ThirdParty/Pix/bin/x64/*.dll")

	list(APPEND FILES_TO_COPY ${FILES_TO_COPY2} ${FILES_TO_COPY3})

	foreach(FILE_TO_COPY IN LISTS FILES_TO_COPY)
		add_custom_command(TARGET AnKiGr POST_BUILD
			COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Binaries
			COMMAND ${CMAKE_COMMAND} -E copy ${FILE_TO_COPY} ${CMAKE_BINARY_DIR}/Binaries)
	endforeach()
endif()
