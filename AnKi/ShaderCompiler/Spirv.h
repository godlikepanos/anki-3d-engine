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

Error doReflectionSpirv(ConstWeakArray<U8> spirv, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr);
/// @}

} // end namespace anki
