# Create AnkiNgx. Inspired by the DLSS example
if(ANKI_DLSS)
	set(NGX_SDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/sdk")
	# Only Linux and Windows support working with Nvidia SDK (NGX)
	if(WINDOWS)
		add_library(AnkiNgx IMPORTED SHARED GLOBAL)
		set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
		set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
		set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_DEBUG ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_d_dbg.lib)
		set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_RELEASE ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_d.lib)
		set_target_properties(AnkiNgx PROPERTIES
			MAP_IMPORTED_CONFIG_MINSIZEREL Release
			MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
		)
		set_target_properties(AnkiNgx PROPERTIES IMPORTED_LOCATION "${NGX_SDK_ROOT}/lib/Windows_x86_64/rel/nvngx_dlss.dll")
	elseif(LINUX)
		add_library(AnkiNgx IMPORTED STATIC GLOBAL)
		set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
		set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
		set_property(TARGET AnkiNgx PROPERTY IMPORTED_LOCATION ${NGX_SDK_ROOT}/lib/Linux_x86_64/libnvsdk_ngx.a)
		# set the list of DLLs that need copying to target folder of application
		set(__NGX_DLLS_LIST "${NGX_SDK_ROOT}/lib/Linux_x86_64/rel/libnvidia-ngx-*.so.*")
	else()
		message(FATAL_ERROR "Trying to use NGX for invalid platform")
	endif()

	set_property(TARGET AnkiNgx APPEND PROPERTY ANKINGX_EXTRA_DLLS "${__NGX_DLLS_LIST}")
endif()
