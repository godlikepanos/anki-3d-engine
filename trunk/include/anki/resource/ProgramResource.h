// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_RESOURCE_H
#define ANKI_RESOURCE_PROGRAM_RESOURCE_H

#include "anki/resource/Common.h"
#include "anki/Gl.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Shader program resource
class ProgramResource
{
public:
	ProgramResource()
	{}

	~ProgramResource()
	{}

	const GlProgramHandle& getGlProgram() const
	{
		return m_prog;
	}

	/// Resource load
	void load(const CString& filename, ResourceInitializer& init);

	/// Load and add extra code on top of the file
	void load(const CString& filename, const CString& extraSrc,
		ResourceManager& manager);

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param filename The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	///        of the shader prog
	/// @param filenamePrefix Add that at the base filename for additional 
	///        ways to identify the file in the cache
	/// @return The file pathname of the new shader prog. Its
	///         $HOME/.anki/cache/ + filenamePrefix + hash + .glsl
	static String createSourceToCache(
		const CString& filename,
		const CString& preAppendedSrcCode,
		const CString& filenamePrefix,
		ResourceManager& manager);

private:
	GlProgramHandle m_prog;
}; 
/// @}

} // end namespace anki

#endif
