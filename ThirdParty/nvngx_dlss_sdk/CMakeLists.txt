if (WIN32)
    set(NGX_USE_STATIC_MSVCRT OFF CACHE BOOL "[Deprecated?]Use NGX libs with static VC runtime (/MT), otherwise dynamic (/MD)")
    set(NGX_SDK_ROOT ${CMAKE_CURRENT_LIST_DIR})

    add_library(AnkiNgx IMPORTED SHARED GLOBAL)

    set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    if(NGX_USE_STATIC_MSVCRT)
        set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_DEBUG ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_s_dbg.lib)
        set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_RELEASE ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_s.lib)
    else()
        set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_DEBUG ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_d_dbg.lib)
        set_target_properties(AnkiNgx PROPERTIES IMPORTED_IMPLIB_RELEASE ${NGX_SDK_ROOT}/lib/Windows_x86_64/x86_64/nvsdk_ngx_d.lib)
    endif()
    set_target_properties(AnkiNgx PROPERTIES
    MAP_IMPORTED_CONFIG_MINSIZEREL Release
    MAP_IMPORTED_CONFIG_RELWITHDEBINFO Release
    )

    # TODO: DO NOT USE/PUSH THE DEV DLLs 
    set_target_properties(AnkiNgx PROPERTIES IMPORTED_LOCATION "${NGX_SDK_ROOT}/lib/Windows_x86_64/rel/nvngx_dlss.dll")
else ()
    add_library(AnkiNgx IMPORTED STATIC GLOBAL)

    set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS RELEASE)
    set_property(TARGET AnkiNgx APPEND PROPERTY IMPORTED_CONFIGURATIONS DEBUG)
    set_target_properties(AnkiNgx PROPERTIES IMPORTED_LOCATION ${NGX_SDK_ROOT}/lib/Linux_x86_64/libnvsdk_ngx.a)
    # TODO: Need debug static library
    # set_target_properties(Ngx PROPERTIES IMPORTED_LOCATION ${NGX_SDK_ROOT}/lib/Linux_x86_64/libnvsdk_ngx.a)

    # set the list of DLLs that need copying to target folder of application
    set(__NGX_DLLS_LIST "${NGX_SDK_ROOT}/lib/Linux_x86_64/$<IF:$<CONFIG:Debug>,dev,rel>/libnvidia-ngx-*.so.*")
endif()

set_property(TARGET AnkiNgx APPEND PROPERTY EXTRA_DLLS "${__NGX_DLLS_LIST}")

target_include_directories(AnkiNgx INTERFACE ${NGX_SDK_ROOT}/include)
