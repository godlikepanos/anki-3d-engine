// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_PROGRAM_RESOURCE_H
#define ANKI_RESOURCE_PROGRAM_RESOURCE_H

#include "anki/resource/ResourceObject.h"
#include "anki/resource/ResourcePointer.h"
#include "anki/Gr.h"

namespace anki {

/// @addtogroup resource
/// @{

/// Shader resource
class ShaderResource: public ResourceObject
{
public:
	ShaderResource(ResourceManager* manager)
		: ResourceObject(manager)
	{}

	~ShaderResource()
	{}

	const ShaderPtr& getGrShader() const
	{
		return m_shader;
	}

	/// Resource load
	ANKI_USE_RESULT Error load(const ResourceFilename& filename);

	/// Load and add extra code on top of the file
	ANKI_USE_RESULT Error load(
		const CString& ResourceFilename, const CString& extraSrc);

	/// Used by @ref Material and @ref Renderer to create custom shaders in
	/// the cache
	/// @param filename The file pathname of the shader prog
	/// @param preAppendedSrcCode The source code we want to write on top
	///        of the shader prog
	/// @param filenamePrefix Add that at the base filename for additional
	///        ways to identify the file in the cache
	/// @param out The file pathname of the new shader prog. It's
	///            filenamePrefix + hash + .glsl
	static ANKI_USE_RESULT Error createToCache(
		const ResourceFilename& filename,
		const CString& preAppendedSrcCode,
		const CString& filenamePrefix,
		ResourceManager& manager,
		StringAuto& out);

	ShaderType getType() const
	{
		return m_type;
	}

private:
	ShaderPtr m_shader;
	ShaderType m_type;
};
/// @}

} // end namespace anki

#endif
