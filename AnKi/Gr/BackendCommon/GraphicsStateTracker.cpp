// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/BackendCommon/GraphicsStateTracker.h>

namespace anki {

Bool GraphicsStateTracker::updateHashes()
{
	Bool someHashWasDirty = false;

	if(m_hashes.m_vert == 0)
	{
		if(m_staticState.m_vert.m_activeAttribs.getAnySet())
		{
			m_hashes.m_vert = 0xC0FEE;

			for(VertexAttributeSemantic i : EnumIterable<VertexAttributeSemantic>())
			{
				if(!m_staticState.m_vert.m_activeAttribs.get(i))
				{
					continue;
				}

				ANKI_ASSERT(m_staticState.m_vert.m_attribsSetMask.get(i) && "Forgot to set the vert attribute");
				m_hashes.m_vert = appendObjectHash(m_staticState.m_vert.m_attribs[i], m_hashes.m_vert);

				ANKI_ASSERT(m_staticState.m_vert.m_bindingsSetMask.get(m_staticState.m_vert.m_attribs[i].m_binding)
							&& "Forgot to inform about the vert binding");
				m_hashes.m_vert = appendObjectHash(m_staticState.m_vert.m_bindings[m_staticState.m_vert.m_attribs[i].m_binding], m_hashes.m_vert);
			}
		}
		else
		{
			m_hashes.m_vert = 0xC0FEE;
		}

		someHashWasDirty = true;
	}

	if(m_hashes.m_rast == 0)
	{
		static_assert(sizeof(m_staticState.m_rast) < kMaxU64);
		memcpy(&m_hashes.m_rast, &m_staticState.m_rast, sizeof(m_staticState.m_rast));
		++m_hashes.m_rast; // Because m_staticState.m_rast may be zero
		someHashWasDirty = true;
	}

	if(m_hashes.m_ia == 0)
	{
		static_assert(sizeof(m_staticState.m_ia) < kMaxU64);
		memcpy(&m_hashes.m_ia, &m_staticState.m_ia, sizeof(m_staticState.m_ia));
		++m_hashes.m_ia; // Because m_staticState.m_ia may be zero
		someHashWasDirty = true;
	}

	if(m_hashes.m_depthStencil == 0)
	{
		const Bool hasStencil =
			m_staticState.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(m_staticState.m_misc.m_depthStencilFormat).isStencil();

		const Bool hasDepth =
			m_staticState.m_misc.m_depthStencilFormat != Format::kNone && getFormatInfo(m_staticState.m_misc.m_depthStencilFormat).isDepth();

		m_hashes.m_depthStencil = 0xC0FEE;

		if(hasStencil)
		{
			m_hashes.m_depthStencil = appendObjectHash(m_staticState.m_stencil, m_hashes.m_depthStencil);
		}

		if(hasDepth)
		{
			m_hashes.m_depthStencil = appendObjectHash(m_staticState.m_depth, m_hashes.m_depthStencil);
		}

		someHashWasDirty = true;
	}

	if(m_hashes.m_blend == 0)
	{
		if(m_staticState.m_misc.m_colorRtMask.getAnySet())
		{
			m_hashes.m_blend = m_staticState.m_blend.m_alphaToCoverage;

			for(U32 i = 0; i < kMaxColorRenderTargets; ++i)
			{
				if(m_staticState.m_misc.m_colorRtMask.get(i))
				{
					m_hashes.m_blend = appendObjectHash(m_staticState.m_blend.m_colorRts[i], m_hashes.m_blend);
				}
			}
		}
		else
		{
			m_hashes.m_blend = 0xC0FFE;
		}

		someHashWasDirty = true;
	}

	if(m_hashes.m_misc == 0)
	{
		m_hashes.m_misc = computeObjectHash(m_staticState.m_misc);
		someHashWasDirty = true;
	}

	if(m_hashes.m_shaderProg == 0)
	{
		m_hashes.m_shaderProg = m_staticState.m_shaderProg->getUuid();
		someHashWasDirty = true;
	}

	if(!someHashWasDirty)
	{
		ANKI_ASSERT(m_globalHash == computeObjectHash(m_hashes));
		return false;
	}
	else
	{
		// Compute complete hash
		const U64 globalHash = computeObjectHash(m_hashes);

		if(globalHash != m_globalHash)
		{
			m_globalHash = globalHash;
			return true;
		}
		else
		{
			return false;
		}
	}
}

} // end namespace anki
