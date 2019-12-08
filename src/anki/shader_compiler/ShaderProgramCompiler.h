// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/shader_compiler/ShaderProgramBinary.h>
#include <anki/util/String.h>
#include <anki/gr/Common.h>

namespace anki
{

/// @addtogroup shader_compiler
/// @{

/// A wrapper over the POD ShaderProgramBinary class.
/// @memberof ShaderProgramCompiler
class ShaderProgramBinaryWrapper : public NonCopyable
{
	friend Error compileShaderProgram(CString fname,
		ShaderProgramFilesystemInterface& fsystem,
		GenericMemoryPoolAllocator<U8> tempAllocator,
		U32 pushConstantsSize,
		U32 backendMinor,
		U32 backendMajor,
		GpuVendor gpuVendor,
		ShaderProgramBinaryWrapper& binary);

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
ANKI_USE_RESULT Error compileShaderProgram(CString fname,
	ShaderProgramFilesystemInterface& fsystem,
	GenericMemoryPoolAllocator<U8> tempAllocator,
	U32 pushConstantsSize,
	U32 backendMinor,
	U32 backendMajor,
	GpuVendor gpuVendor,
	ShaderProgramBinaryWrapper& binary);

/// Create a human readable representation of the shader binary.
void disassembleShaderProgramBinary(const ShaderProgramBinary& binary, StringAuto& humanReadable);
/// @}

} // end namespace anki
