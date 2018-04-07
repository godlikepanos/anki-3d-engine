// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>
#include <anki/util/DynamicArray.h>

namespace anki
{

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
	class BuildContext;

	ShaderCompiler(GenericMemoryPoolAllocator<U8> alloc);

	~ShaderCompiler();

	/// Compile a shader.
	/// @param source The source in GLSL.
	/// @param options Compile options.
	/// @param bin The output binary.
	ANKI_USE_RESULT Error compile(
		CString source, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const;

anki_internal:
	static void logShaderErrorCode(CString error, CString source, GenericMemoryPoolAllocator<U8> alloc);

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	static I32 m_refcount;
	static Mutex m_refcountMtx;
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
	ANKI_USE_RESULT Error compile(
		CString source, U64* hash, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const;

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	ShaderCompiler m_compiler;
	String m_cacheDir;

	ANKI_USE_RESULT Error compileInternal(
		CString source, U64* hash, const ShaderCompilerOptions& options, DynamicArrayAuto<U8>& bin) const;
};
/// @}

} // end namespace anki
