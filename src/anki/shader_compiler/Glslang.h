// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/Common.h>
#include <anki/gr/Enums.h>

#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic push
#	pragma GCC diagnostic ignored "-Wundef"
#	pragma GCC diagnostic ignored "-Wconversion"
#endif
#include <glslang/Public/ShaderLang.h>
#include <glslang/SPIRV/GlslangToSpv.h>
#include <glslang/StandAlone/DirStackFileIncluder.h>
#if ANKI_COMPILER_GCC_COMPATIBLE
#	pragma GCC diagnostic pop
#endif

namespace anki
{

/// @addtogroup shader_compiler
/// @{
extern TBuiltInResource GLSLANG_LIMITS;

EShLanguage ankiToGlslangShaderType(ShaderType shaderType);
/// @}

} // end namespace anki
