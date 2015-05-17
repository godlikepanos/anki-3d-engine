// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_GR_GL_RESOURCE_GROUP_IMPL_H
#define ANKI_GR_GL_RESOURCE_GROUP_IMPL_H

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/ResourceGroupCommon.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Resource group implementation.
class ResourceGroupImpl: public GlObject, private ResourceGroupInitializer
{
public:
	using Base = GlObject;
	using Initializer = ResourceGroupInitializer;

	ResourceGroupImpl(GrManager* manager)
	:	Base(manager)
	{}

	~ResourceGroupImpl()
	{}

	ANKI_USE_RESULT Error create(const Initializer& init);

	/// Set state.
	void bind();

private:
	Array<GLuint, MAX_TEXTURE_BINDINGS> m_textureNames;
	Array<GLuint, MAX_TEXTURE_BINDINGS> m_samplerNames;
	U8 m_textureNamesCount = 0;
	Bool8 m_allSamplersZero = false;

	U8 m_ubosCount = 0;
	U8 m_ssbosCount = 0;

	Array<GLuint, MAX_VERTEX_ATTRIBUTES> m_vertBuffNames;
	Array<GLintptr, MAX_VERTEX_ATTRIBUTES> m_vertBuffOffsets;
	U8 m_vertBindingsCount = 0;
};
/// @}

} // end namespace anki

#endif
