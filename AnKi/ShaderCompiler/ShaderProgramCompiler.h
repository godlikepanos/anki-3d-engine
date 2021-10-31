// Copyright (C) 2009-2021, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/ShaderProgramDump.h>
#include <AnKi/Util/String.h>
#include <AnKi/Gr/Common.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

constexpr const char* SHADER_BINARY_MAGIC = "ANKISDR7"; ///< WARNING: If changed change SHADER_BINARY_VERSION
constexpr U32 SHADER_BINARY_VERSION = 7;

/// A wrapper over the POD ShaderProgramBinary class.
/// @memberof ShaderProgramCompiler
class ShaderProgramBinaryWrapper
{
	friend Error compileShaderProgramInternal(CString fname, ShaderProgramFilesystemInterface& fsystem,
											  ShaderProgramPostParseInterface* postParseCallback,
											  ShaderProgramAsyncTaskInterface* taskManager,
											  GenericMemoryPoolAllocator<U8> tempAllocator,
											  const ShaderCompilerOptions& compilerOptions,
											  ShaderProgramBinaryWrapper& binary);

public:
	ShaderProgramBinaryWrapper(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	ShaderProgramBinaryWrapper(const ShaderProgramBinaryWrapper&) = delete; // Non-copyable

	~ShaderProgramBinaryWrapper()
	{
		cleanup();
	}

	ShaderProgramBinaryWrapper& operator=(const ShaderProgramBinaryWrapper&) = delete; // Non-copyable

	ANKI_USE_RESULT Error serializeToFile(CString fname) const;

	ANKI_USE_RESULT Error deserializeFromFile(CString fname);

	const ShaderProgramBinary& getBinary() const
	{
		ANKI_ASSERT(m_binary);
		return *m_binary;
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	ShaderProgramBinary* m_binary = nullptr;
	Bool m_singleAllocation = false;

	void cleanup();
};

/// Takes an AnKi special shader program and spits a binary.
ANKI_USE_RESULT Error compileShaderProgram(CString fname, ShaderProgramFilesystemInterface& fsystem,
										   ShaderProgramPostParseInterface* postParseCallback,
										   ShaderProgramAsyncTaskInterface* taskManager,
										   GenericMemoryPoolAllocator<U8> tempAllocator,
										   const ShaderCompilerOptions& compilerOptions,
										   ShaderProgramBinaryWrapper& binary);
/// @}

} // end namespace anki
