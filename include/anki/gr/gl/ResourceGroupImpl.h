// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/gr/gl/GlObject.h"
#include "anki/gr/ResourceGroup.h"

namespace anki {

/// @addtogroup opengl
/// @{

/// Resource group implementation.
class ResourceGroupImpl: public GlObject
{
public:
	ResourceGroupImpl(GrManager* manager)
		: GlObject(manager)
	{}

	~ResourceGroupImpl()
	{}

	void create(const ResourceGroupInitializer& init);

	/// Set state.
	void bind(GlState& state);

private:
	ResourceGroupInitializer m_in; ///< That will hold the references

	class
	{
	public:
		Array<GLuint, MAX_TEXTURE_BINDINGS> m_textureNames;
		Array<GLuint, MAX_TEXTURE_BINDINGS> m_samplerNames;
		U8 m_textureNamesCount = 0;
		Bool8 m_allSamplersZero = false;

		U8 m_ubosCount = 0;
		U8 m_ssbosCount = 0;

		Array<GLuint, MAX_VERTEX_ATTRIBUTES> m_vertBuffNames;
		Array<GLintptr, MAX_VERTEX_ATTRIBUTES> m_vertBuffOffsets;
		U8 m_vertBindingsCount = 0;
	} m_cache;
};
/// @}

} // end namespace anki

