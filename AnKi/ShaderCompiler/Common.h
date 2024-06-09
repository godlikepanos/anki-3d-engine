// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#define ANKI_SHADER_COMPILER_LOGV(...) ANKI_LOG("SHCO", kVerbose, __VA_ARGS__)

class ShaderCompilerMemoryPool : public HeapMemoryPool, public MakeSingleton<ShaderCompilerMemoryPool>
{
	template<typename>
	friend class MakeSingleton;

private:
	ShaderCompilerMemoryPool(AllocAlignedCallback allocCb, void* allocCbUserData)
		: HeapMemoryPool(allocCb, allocCbUserData, "ShaderCompilerMemPool")
	{
	}

	~ShaderCompilerMemoryPool() = default;
};

ANKI_DEFINE_SUBMODULE_UTIL_CONTAINERS(ShaderCompiler, ShaderCompilerMemoryPool)

constexpr U32 kMaxShaderBinaryNameLength = 127;

using MutatorValue = I32; ///< The type of the mutator value

/// An interface used by the ShaderParser and compileShaderProgram to abstract file loading.
class ShaderCompilerFilesystemInterface
{
public:
	virtual Error readAllText(CString filename, ShaderCompilerString& txt) = 0;
};

/// This controls if the compilation will continue after the parsing stage.
class ShaderCompilerPostParseInterface
{
public:
	virtual Bool skipCompilation(U64 programHash) = 0;
};

/// An interface for asynchronous shader compilation.
class ShaderCompilerAsyncTaskInterface
{
public:
	virtual void enqueueTask(void (*callback)(void* userData), void* userData) = 0;

	virtual Error joinTasks() = 0;
};

class ShaderCompilerDefine
{
public:
	CString m_name;
	I32 m_value;
};
/// @}

} // end namespace anki
