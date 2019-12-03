// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/util/Logger.h>
#include <anki/util/String.h>
#include <anki/util/BitSet.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

constexpr U32 MAX_SHADER_PROGRAM_INPUT_VARIABLES = 128;
constexpr U32 MAX_SHADER_BINARY_NAME_LENGTH = 63;

using ActiveProgramInputVariableMask = BitSet<MAX_SHADER_PROGRAM_INPUT_VARIABLES, U64>;

#define ANKI_SHADER_COMPILER_LOGI(...) ANKI_LOG(" SHC", NORMAL, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGE(...) ANKI_LOG(" SHC", ERROR, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGW(...) ANKI_LOG(" SHC", WARNING, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGF(...) ANKI_LOG(" SHC", FATAL, __VA_ARGS__)

/// An interface used by the ShaderProgramParser and ShaderProgramCompiler to abstract file loading.
class ShaderProgramFilesystemInterface
{
public:
	virtual ANKI_USE_RESULT Error readAllText(CString filename, StringAuto& txt) = 0;
};
/// @}

} // end namespace anki
