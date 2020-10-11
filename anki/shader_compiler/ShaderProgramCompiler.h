// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/ShaderProgramDump.h>
#include <anki/util/String.h>
#include <anki/gr/Common.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

extern const U32 SHADER_BINARY_VERSION;

/// A wrapper over the POD ShaderProgramBinary class.
/// @memberof ShaderProgramCompiler
class ShaderProgramBinaryWrapper : public NonCopyable
{
	friend Error compileShaderProgramInternal(CString fname, ShaderProgramFilesystemInterface& fsystem,
											  ShaderProgramPostParseInterface* postParseCallback,
											  ShaderProgramAsyncTaskInterface* taskManager,
											  GenericMemoryPoolAllocator<U8> tempAllocator,
											  const GpuDeviceCapabilities& gpuCapabilities,
											  const BindlessLimits& bindlessLimits, ShaderProgramBinaryWrapper& binary);

public:
	ShaderProgramBinaryWrapper(GenericMemoryPoolAllocator<U8> alloc)
		: m_alloc(alloc)
	{
	}

	~ShaderProgramBinaryWrapper()
	{
		cleanup();
	}

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
										   const GpuDeviceCapabilities& gpuCapabilities,
										   const BindlessLimits& bindlessLimits, ShaderProgramBinaryWrapper& binary);
/// @}

} // end namespace anki
