// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/Pipeline.h>

namespace anki
{

void PipelineStateTracker::updateHashes()
{
	Bool stateDirty = false;

	// Vertex
	if(m_dirty.m_attribs.getAny() || m_dirty.m_vertBindings.getAny())
	{
		for(U i = 0; i < MAX_VERTEX_ATTRIBUTES; ++i)
		{
			if(m_shaderAttributeMask.get(i))
			{
				ANKI_ASSERT(m_set.m_attribs.get(i) && "Forgot to set the attribute");

				Bool dirty = false;
				if(m_dirty.m_attribs.get(i))
				{
					m_dirty.m_attribs.unset(i);
					dirty = true;
				}

				const U binding = m_state.m_vertex.m_attributes[i].m_binding;
				if(m_dirty.m_vertBindings.get(binding))
				{
					m_dirty.m_vertBindings.unset(binding);
					dirty = true;
				}

				if(dirty)
				{
					m_hashes.m_vertexAttribs[i] =
						computeHash(&m_state.m_vertex.m_attributes[i], sizeof(m_state.m_vertex.m_attributes[i]));
					m_hashes.m_vertexAttribs[i] = appendHash(&m_state.m_vertex.m_bindings[i],
						sizeof(m_state.m_vertex.m_bindings[i]),
						m_hashes.m_vertexAttribs[i]);

					stateDirty = true;
				}
			}
		}
	}

	// IA
	if(m_dirty.m_ia)
	{
		m_dirty.m_ia = false;
		m_hashes.m_ia = computeHash(&m_state.m_inputAssembler, sizeof(m_state.m_inputAssembler));
		stateDirty = true;
	}

	// Rasterizer
	if(m_dirty.m_raster)
	{
		m_dirty.m_raster = false;
		stateDirty = true;
		m_hashes.m_raster = computeHash(&m_state.m_rasterizer, sizeof(m_state.m_rasterizer));
	}

	// Depth
	if(m_dirty.m_depth && m_fbDepth)
	{
		m_dirty.m_depth = false;
		stateDirty = true;
		m_hashes.m_depth = computeHash(&m_state.m_depth, sizeof(m_state.m_depth));
	}

	// Stencil
	if(m_dirty.m_stencil && m_fbStencil)
	{
		m_dirty.m_stencil = false;
		stateDirty = true;
		m_hashes.m_stencil = computeHash(&m_state.m_stencil, sizeof(m_state.m_stencil));
	}

	// Color
	if(m_fbColorAttachmentMask.getAny())
	{
		ANKI_ASSERT(m_fbColorAttachmentMask == m_shaderColorAttachmentWritemask
			&& "Shader and fb should have same attachment mask");
	}

	if(m_dirty.m_color || !!(m_dirty.m_colAttachments & m_shaderColorAttachmentWritemask))
	{
		if(m_dirty.m_color)
		{
			m_hashes.m_color = m_state.m_color.m_alphaToCoverageEnabled ? 1 : 2;
			stateDirty = true;
		}

		for(U i = 0; i < MAX_COLOR_ATTACHMENTS; ++i)
		{
			if(m_shaderColorAttachmentWritemask.get(i) && m_dirty.m_colAttachments.get(i))
			{
				m_hashes.m_colAttachments[i] =
					computeHash(&m_state.m_color.m_attachments[i], sizeof(m_state.m_color.m_attachments[i]));
				stateDirty = true;
			}
		}
	}
}

} // end namespace anki
