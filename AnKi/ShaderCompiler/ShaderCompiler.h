// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/ShaderCompiler/ShaderBinary.h>
#include <AnKi/Util/String.h>
#include <AnKi/Gr/Common.h>

namespace anki {

/// @addtogroup shader_compiler
/// @{

inline constexpr const char* kShaderBinaryMagic = "ANKISP1"; // WARNING: If changed change kShaderBinaryVersion
constexpr U32 kShaderBinaryVersion = 1;

template<typename TFile>
Error deserializeShaderBinaryFromAnyFile(TFile& file, ShaderBinary*& binary, BaseMemoryPool& pool)
{
	BinaryDeserializer deserializer;
	ANKI_CHECK(deserializer.deserialize(binary, pool, file));
	if(memcmp(kShaderBinaryMagic, &binary->m_magic[0], strlen(kShaderBinaryMagic)) != 0)
	{
		ANKI_SHADER_COMPILER_LOGE("Corrupted or wrong version of shader binary.");
		return Error::kUserData;
	}

	return Error::kNone;
}

inline Error deserializeShaderBinaryFromFile(CString fname, ShaderBinary*& binary, BaseMemoryPool& pool)
{
	File file;
	ANKI_CHECK(file.open(fname, FileOpenFlag::kRead | FileOpenFlag::kBinary));
	ANKI_CHECK(deserializeShaderBinaryFromAnyFile(file, binary, pool));
	return Error::kNone;
}

/// Takes an AnKi special shader program and spits a binary.
Error compileShaderProgram(CString fname, Bool spirv, Bool debugInfo, ShaderCompilerFilesystemInterface& fsystem,
						   ShaderCompilerPostParseInterface* postParseCallback, ShaderCompilerAsyncTaskInterface* taskManager,
						   ConstWeakArray<ShaderCompilerDefine> defines, ShaderBinary*& binary);

/// Free the binary created ONLY by compileShaderProgram.
void freeShaderBinary(ShaderBinary*& binary);

Error doReflectionDxil(ConstWeakArray<U8> dxil, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr);
Error doReflectionSpirv(ConstWeakArray<U8> spirv, ShaderType type, ShaderReflection& refl, ShaderCompilerString& errorStr);
/// @}

} // end namespace anki
