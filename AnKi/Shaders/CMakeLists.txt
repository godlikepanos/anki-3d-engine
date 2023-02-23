file(GLOB_RECURSE prog_fnames *.ankiprog)

ProcessorCount(proc_count)
MATH(EXPR proc_count "${proc_count}-1")

if(ANKI_OVERRIDE_SHADER_COMPILER STREQUAL "")
	set(shader_compiler_bin "ShaderCompiler")
	set(shader_compiler_dep "ShaderCompiler")
else()
	set(shader_compiler_bin ${ANKI_OVERRIDE_SHADER_COMPILER})
	set(shader_compiler_dep "")
endif()

if(ANDROID)
	# Copy to the Android assets
	set(out_dir "${CMAKE_BINARY_DIR}/../../../../../assets/ShaderBinaries")
else()
	set(out_dir "${CMAKE_BINARY_DIR}/Binaries/ShaderBinaries")
endif()

if(ANDROID OR ARM)
	message("++ Compiling shaders for mobile")
	set(extra_compiler_args "-mobile-platform")
else()
	message("++ Compiling shaders for desktop")
endif()

if(ANKI_SHADER_FULL_PRECISION)
	message("++ Forcing full shader precision")
	set(extra_compiler_args ${extra_compiler_args} "-force-full-fp")
else()
	message("++ Leaving default shader precision")
endif()

include(FindPythonInterp)

foreach(prog_fname ${prog_fnames})
	get_filename_component(filename ${prog_fname} NAME)
	set(bin_fname ${CMAKE_CURRENT_BINARY_DIR}/${filename}bin)

	get_filename_component(filename2 ${prog_fname} NAME_WE)
	set(target_name "${filename2}_ankiprogbin")

	# Get deps using a script
	execute_process(
		COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_CURRENT_SOURCE_DIR}/../../Tools/Shader/ShaderProgramDependencies.py" "-i" "AnKi/Shaders/${filename}"
		WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../.."
		OUTPUT_VARIABLE deps)

	add_custom_command(
		OUTPUT ${bin_fname}
		COMMAND ${shader_compiler_bin} -o ${bin_fname} -j ${proc_count} -I "${CMAKE_CURRENT_SOURCE_DIR}/../.." ${extra_compiler_args} ${prog_fname}
		DEPENDS ${shader_compiler_dep} ${prog_fname} ${deps}
		COMMENT "Build ${prog_fname}")

	add_custom_target(
		${target_name} ALL
		DEPENDS ${bin_fname})

	add_custom_command(
		TARGET ${target_name} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${out_dir}
		COMMAND ${CMAKE_COMMAND} -E copy ${bin_fname} ${out_dir})

	list(APPEND program_targets ${target_name})
endforeach()

add_custom_target(AnKiShaders ALL DEPENDS ${program_targets})
