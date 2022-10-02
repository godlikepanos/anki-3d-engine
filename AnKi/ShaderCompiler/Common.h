// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Logger.h>
#include <AnKi/Util/String.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

#define ANKI_SHADER_COMPILER_LOGI(...) ANKI_LOG("SHCO", kNormal, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGE(...) ANKI_LOG("SHCO", kError, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGW(...) ANKI_LOG("SHCO", kWarning, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGF(...) ANKI_LOG("SHCO", kFatal, __VA_ARGS__)

constexpr U32 kMaxShaderBinaryNameLength = 127;

using MutatorValue = I32; ///< The type of the mutator value

/// An interface used by the ShaderProgramParser and ShaderProgramCompiler to abstract file loading.
class ShaderProgramFilesystemInterface
{
public:
	virtual Error readAllText(CString filename, StringRaii& txt) = 0;
};

/// This controls if the compilation will continue after the parsing stage.
class ShaderProgramPostParseInterface
{
public:
	virtual Bool skipCompilation(U64 programHash) = 0;
};

/// An interface for asynchronous shader compilation.
class ShaderProgramAsyncTaskInterface
{
public:
	virtual void enqueueTask(void (*callback)(void* userData), void* userData) = 0;

	virtual Error joinTasks() = 0;
};

/// Options to be passed to the compiler.
ANKI_BEGIN_PACKED_STRUCT
class ShaderCompilerOptions
{
public:
	Bool m_forceFullFloatingPointPrecision = false;
	Bool m_mobilePlatform = false;
};
ANKI_END_PACKED_STRUCT
/// @}

} // end namespace anki
