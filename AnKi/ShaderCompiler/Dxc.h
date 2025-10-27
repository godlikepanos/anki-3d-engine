// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

inline constexpr Array2d<U32, kMaxRegisterSpaces, U32(HlslResourceType::kCount)> kDxcVkBindingShifts = {
	{{1000, 2000, 3000, 4000}, {5000, 6000, 7000, 8000}, {9000, 10000, 11000, 12000}}};

/// Compile HLSL to SPIR-V.
Error compileHlslToSpirv(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ShaderModel sm,
						 ConstWeakArray<CString> compilerArgs, ShaderCompilerDynamicArray<U8>& spirv, ShaderCompilerString& errorMessage);

/// Compile HLSL to DXIL.
Error compileHlslToDxil(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ShaderModel sm,
						ConstWeakArray<CString> compilerArgs, ShaderCompilerDynamicArray<U8>& dxil, ShaderCompilerString& errorMessage);

Error doReflectionDxil(ConstWeakArray<U8> dxil, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorMessage);
/// @}

} // end namespace anki
