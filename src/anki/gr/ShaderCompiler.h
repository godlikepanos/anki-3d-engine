// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/DynamicArray.h>

namespace anki
{

// Forward
class StringList;

/// @addtogroup graphics
/// @{

enum class ShaderLanguage : U8
{
	GLSL,
	SPIRV,
	COUNT
};

/// ShaderCompiler compile options.
class ShaderCompilerOptions
{
public:
	ShaderLanguage m_outLanguage;
	ShaderType m_shaderType;
	GpuDeviceCapabilities m_gpuCapabilities;

	ShaderCompilerOptions()
	{
		// Zero it because it will be hashed
		zeroMemory(*this);
		m_outLanguage = ShaderLanguage::COUNT;
		m_shaderType = ShaderType::COUNT;
		::new(&m_gpuCapabilities) GpuDeviceCapabilities();
	}

	void setFromGrManager(const GrManager& gr);
};

/// Shader compiler. It's backend agnostic.
class ShaderCompiler
{
public:
	ShaderCompiler(GenericMemoryPoolAllocator<U8> alloc);

	~ShaderCompiler();

	/// Compile a shader.
	/// @param source The source in GLSL.
	/// @param options Compile options.
	/// @param bin The output binary.
	/// @param finalSourceDumpFilename Write the final source to this filename if it's not empty.
	ANKI_USE_RESULT Error compile(CString source,
		const ShaderCompilerOptions& options,
		DynamicArrayAuto<U8>& bin,
		CString finalSourceDumpFilename = {}) const;

	ANKI_USE_RESULT Error preprocess(
		CString source, const ShaderCompilerOptions& options, const StringList& defines, StringAuto& out) const;

anki_internal:
	static void logShaderErrorCode(CString error, CString source, GenericMemoryPoolAllocator<U8> alloc);

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	static I32 m_refcount;
	static Mutex m_refcountMtx;

	ANKI_USE_RESULT Error genSpirv(
		CString src, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& spirv) const;

	ANKI_USE_RESULT Error preprocessCommon(CString in, const ShaderCompilerOptions& options, StringAuto& out) const;
};

/// Like ShaderCompiler but on steroids. It uses a cache to avoid compiling shaders else it calls
/// ShaderCompiler::compile
class ShaderCompilerCache
{
public:
	ShaderCompilerCache(GenericMemoryPoolAllocator<U8> alloc, CString cacheDir)
		: m_alloc(alloc)
		, m_compiler(alloc)
	{
		ANKI_ASSERT(!cacheDir.isEmpty());
		m_cacheDir.create(alloc, cacheDir);
	}

	~ShaderCompilerCache()
	{
		m_cacheDir.destroy(m_alloc);
	}

	/// Compile a shader.
	/// @param source The source in GLSL.
	/// @param sourceHash Optional hash of the source. If it's nullptr then the @a source will be hashed.
	/// @param options Compile options.
	/// @param bin The output binary.
	/// @param dumpShaderSource If true dump the shaders' source as well.
	ANKI_USE_RESULT Error compile(CString source,
		U64* hash,
		const ShaderCompilerOptions& options,
		DynamicArrayAuto<U8>& bin,
		Bool dumpShaderSource = false) const;

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	ShaderCompiler m_compiler;
	String m_cacheDir;

	ANKI_USE_RESULT Error compileInternal(CString source,
		U64* hash,
		const ShaderCompilerOptions& options,
		DynamicArrayAuto<U8>& bin,
		Bool dumpShaderSource) const;
};
/// @}

} // end namespace anki
