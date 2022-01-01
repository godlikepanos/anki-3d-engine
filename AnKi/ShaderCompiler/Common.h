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

#define ANKI_SHADER_COMPILER_LOGI(...) ANKI_LOG("SHCO", NORMAL, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGE(...) ANKI_LOG("SHCO", ERROR, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGW(...) ANKI_LOG("SHCO", WARNING, __VA_ARGS__)
#define ANKI_SHADER_COMPILER_LOGF(...) ANKI_LOG("SHCO", FATAL, __VA_ARGS__)

constexpr U32 MAX_SHADER_BINARY_NAME_LENGTH = 127;

using MutatorValue = I32; ///< The type of the mutator value

/// An interface used by the ShaderProgramParser and ShaderProgramCompiler to abstract file loading.
class ShaderProgramFilesystemInterface
{
public:
	virtual ANKI_USE_RESULT Error readAllText(CString filename, StringAuto& txt) = 0;
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

	virtual ANKI_USE_RESULT Error joinTasks() = 0;
};

/// Bindless related info.
ANKI_BEGIN_PACKED_STRUCT
class BindlessLimits
{
public:
	U32 m_bindlessTextureCount = 0;
	U32 m_bindlessImageCount = 0;
};
ANKI_END_PACKED_STRUCT

/// Options to be passed to the compiler.
ANKI_BEGIN_PACKED_STRUCT
class ShaderCompilerOptions
{
public:
	BindlessLimits m_bindlessLimits;
	Bool m_forceFullFloatingPointPrecision = false;
};
ANKI_END_PACKED_STRUCT
/// @}

} // end namespace anki
