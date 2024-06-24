// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/Common.h>
#include <AnKi/Util/String.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

inline constexpr Array2d<U32, kMaxDescriptorSets, U32(HlslResourceType::kCount)> kDxcVkBindingShifts = {
	{{1000, 2000, 3000, 4000}, {5000, 6000, 7000, 8000}, {9000, 10000, 11000, 12000}}};

// !!!!WARNING!!!! Need to change HLSL if you change the value bellow
inline constexpr U32 kDxcVkBindlessRegisterSpace = 1000000;

/// Compile HLSL to SPIR-V.
Error compileHlslToSpirv(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ShaderCompilerDynamicArray<U8>& spirv,
						 ShaderCompilerString& errorMessage);

/// Compile HLSL to DXIL.
Error compileHlslToDxil(CString src, ShaderType shaderType, Bool compileWith16bitTypes, Bool debugInfo, ShaderCompilerDynamicArray<U8>& dxil,
						ShaderCompilerString& errorMessage);
/// @}

} // end namespace anki
