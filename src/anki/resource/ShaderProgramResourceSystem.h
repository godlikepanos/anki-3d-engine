// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/resource/Common.h>

namespace anki
{

/// @addtogroup resource
/// @{

/// A system that does some work with shader programs.
class ShaderProgramResourceSystem
{
public:
	ShaderProgramResourceSystem(CString cacheDir, GrManager* gr, ResourceFilesystem* fs,
								const GenericMemoryPoolAllocator<U8>& alloc)
		: m_alloc(alloc)
		, m_gr(gr)
		, m_fs(fs)
	{
		m_cacheDir.create(alloc, cacheDir);
	}

	~ShaderProgramResourceSystem()
	{
		m_cacheDir.destroy(m_alloc);
	}

	ANKI_USE_RESULT Error init()
	{
		return compileAllShaders(m_cacheDir, *m_gr, *m_fs, m_alloc);
	}

private:
	GenericMemoryPoolAllocator<U8> m_alloc;
	String m_cacheDir;
	GrManager* m_gr;
	ResourceFilesystem* m_fs;

	/// Iterate all programs in the filesystem and compile them to AnKi's binary format.
	static Error compileAllShaders(CString cacheDir, GrManager& gr, ResourceFilesystem& fs,
								   GenericMemoryPoolAllocator<U8>& alloc);
};
/// @}

} // end namespace anki
