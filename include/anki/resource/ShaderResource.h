// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_RESOURCE_H
#define ANKI_RESOURCE_PROGRAM_RESOURCE_H

#include "anki/resource/Common.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Shader resource
class ShaderResource
{
public:
	ShaderResource(ResourceAllocator<U8>&)
	{}

	~ShaderResource()
	{}

	const ShaderHandle& getGrShader() const
	{
		return m_shader;
	}

	/// Resource load
	ANKI_USE_RESULT Error load(
		const CString& filename, ResourceInitializer& init);

	/// Load and add extra code on top of the file
	ANKI_USE_RESULT Error load(
		const CString& filename, const CString& extraSrc,
		ResourceManager& manager);

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param filename The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	///        of the shader prog
	/// @param filenamePrefix Add that at the base filename for additional 
	///        ways to identify the file in the cache
	/// @param out The file pathname of the new shader prog. Its
	///            $HOME/.anki/cache/ + filenamePrefix + hash + .glsl
	static ANKI_USE_RESULT Error createToCache(
		const CString& filename,
		const CString& preAppendedSrcCode,
		const CString& filenamePrefix,
		ResourceManager& manager,
		TempResourceString& out);

private:
	ShaderHandle m_shader;
}; 
/// @}

} // end namespace anki

#endif
