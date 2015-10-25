// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/gl/GlObject.h>
#include <anki/gr/ResourceGroup.h>
#include <anki/util/DArray.h>

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
	{
		m_refs.destroy(getAllocator());
	}

	void create(const ResourceGroupInitializer& init);

	/// Set state.
	void bind(U slot, const DynamicBufferInfo& dynInfo, GlState& state);

private:
	class InternalBufferBinding
	{
	public:
		GLuint m_name = 0; ///< If it's MAX_U32 then it's dynamic
		U32 m_offset = 0;
		U32 m_range = 0;
	};

	Array<GLuint, MAX_TEXTURE_BINDINGS> m_textureNames;
	Array<GLuint, MAX_TEXTURE_BINDINGS> m_samplerNames;
	U8 m_textureNamesCount = 0;
	Bool8 m_allSamplersZero = false;

	Array<InternalBufferBinding, MAX_UNIFORM_BUFFER_BINDINGS> m_ubos;
	U8 m_ubosCount = 0;

	Array<InternalBufferBinding, MAX_STORAGE_BUFFER_BINDINGS> m_ssbos;
	U8 m_ssbosCount = 0;

	Array<GLuint, MAX_VERTEX_ATTRIBUTES> m_vertBuffNames;
	Array<GLintptr, MAX_VERTEX_ATTRIBUTES> m_vertBuffOffsets;
	Bool8 m_hasDynamicVertexBuff = false;
	U8 m_vertBindingsCount = 0;

	GLuint m_indexBuffName = 0;
	U8 m_indexSize = 0;

	/// Holds the references to the resources. Used to release the references
	/// gracefully
	DArray<IntrusivePtr<GrObject>> m_refs;

	template<typename InBindings, typename OutBindings>
	void initBuffers(const InBindings& in, OutBindings& out, U8& count,
		U& resourcesCount, U& dynCount);

	void initResourceReferences(const ResourceGroupInitializer& init, U count);
};
/// @}

} // end namespace anki

