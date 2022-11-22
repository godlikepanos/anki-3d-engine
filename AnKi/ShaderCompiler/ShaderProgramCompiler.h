// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/ShaderProgramDump.h>
#include <AnKi/Util/String.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

inline constexpr const char* kShaderBinaryMagic = "ANKISDR8"; //! WARNING: If changed change kShaderBinaryVersion
constexpr U32 kShaderBinaryVersion = 8;

/// A wrapper over the POD ShaderProgramBinary class.
/// @memberof ShaderProgramCompiler
class ShaderProgramBinaryWrapper
{
	friend Error compileShaderProgramInternal(CString fname, ShaderProgramFilesystemInterface& fsystem,
											  ShaderProgramPostParseInterface* postParseCallback,
											  ShaderProgramAsyncTaskInterface* taskManager, BaseMemoryPool& tempPool,
											  const ShaderCompilerOptions& compilerOptions,
											  ShaderProgramBinaryWrapper& binary);

public:
	ShaderProgramBinaryWrapper(BaseMemoryPool* pool)
		: m_pool(pool)
	{
	}

	ShaderProgramBinaryWrapper(const ShaderProgramBinaryWrapper&) = delete; // Non-copyable

	~ShaderProgramBinaryWrapper()
	{
		cleanup();
	}

	ShaderProgramBinaryWrapper& operator=(const ShaderProgramBinaryWrapper&) = delete; // Non-copyable

	Error serializeToFile(CString fname) const;

	Error deserializeFromFile(CString fname);

	template<typename TFile>
	Error deserializeFromAnyFile(TFile& fname);

	const ShaderProgramBinary& getBinary() const
	{
		ANKI_ASSERT(m_binary);
		return *m_binary;
	}

private:
	BaseMemoryPool* m_pool = nullptr;
	ShaderProgramBinary* m_binary = nullptr;
	Bool m_singleAllocation = false;

	void cleanup();
};

template<typename TFile>
Error ShaderProgramBinaryWrapper::deserializeFromAnyFile(TFile& file)
{
	cleanup();
	BinaryDeserializer deserializer;
	ANKI_CHECK(deserializer.deserialize(m_binary, *m_pool, file));

	m_singleAllocation = true;

	if(memcmp(kShaderBinaryMagic, &m_binary->m_magic[0], strlen(kShaderBinaryMagic)) != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Corrupted or wrong version of shader binary.");
		return Error::kUserData;
	}

	return Error::kNone;
}

/// Takes an AnKi special shader program and spits a binary.
Error compileShaderProgram(CString fname, ShaderProgramFilesystemInterface& fsystem,
						   ShaderProgramPostParseInterface* postParseCallback,
						   ShaderProgramAsyncTaskInterface* taskManager, BaseMemoryPool& tempPool,
						   const ShaderCompilerOptions& compilerOptions, ShaderProgramBinaryWrapper& binary);
/// @}

} // end namespace anki
