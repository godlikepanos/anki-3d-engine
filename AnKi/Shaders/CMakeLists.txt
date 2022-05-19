file(GLOB_RECURSE prog_fnames *.ankiprog)
file(GLOB_RECURSE glsl_fnames *.glsl)
file(GLOB_RECURSE header_fnames *.h)

ProcessorCount(proc_count)
MATH(EXPR proc_count "${proc_count}-1")

foreach(prog_fname ${prog_fnames})
	get_filename_component(filename ${prog_fname} NAME)
	set(bin_fname ${CMAKE_CURRENT_BINARY_DIR}/${filename}bin)

	get_filename_component(filename ${prog_fname} NAME_WE)
	set(target_name "${filename}_ankiprogbin")

	add_custom_command(
		OUTPUT ${bin_fname}
		COMMAND ShaderCompiler -o ${bin_fname} -j ${proc_count} -I "${CMAKE_CURRENT_SOURCE_DIR}/../.." ${prog_fname}
		DEPENDS ShaderCompiler ${prog_fname} ${glsl_fnames} ${header_fnames}
		COMMENT "Build ${prog_fname}")

	add_custom_target(
		${target_name} ALL
		DEPENDS ${bin_fname})

	add_custom_command(
		TARGET ${target_name} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/Bin/ShaderBinaries
		COMMAND ${CMAKE_COMMAND} -E copy ${bin_fname} ${CMAKE_BINARY_DIR}/Bin/ShaderBinaries)

	list(APPEND program_targets ${target_name})
endforeach()

add_custom_target(AnKiShaders DEPENDS ${program_targets})
