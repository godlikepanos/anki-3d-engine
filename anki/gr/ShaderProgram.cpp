// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/ShaderProgram.h>

namespace anki
{

Bool ShaderProgramInitInfo::isValid() const
{
	ShaderTypeBit graphicsMask = ShaderTypeBit::NONE;
	for(ShaderType i = ShaderType::FIRST_GRAPHICS; i <= ShaderType::LAST_GRAPHICS; ++i)
	{
		if(m_graphicsShaders[i])
		{
			if(m_graphicsShaders[i]->getShaderType() != i)
			{
				return false;
			}
			graphicsMask |= ShaderTypeBit(1 << i);
		}
	}

	if(!!graphicsMask
	   && (graphicsMask & (ShaderTypeBit::VERTEX | ShaderTypeBit::FRAGMENT))
			  != (ShaderTypeBit::VERTEX | ShaderTypeBit::FRAGMENT))
	{
		return false;
	}

	Bool compute = false;
	if(m_computeShader)
	{
		if(m_computeShader->getShaderType() != ShaderType::COMPUTE)
		{
			return false;
		}
		compute = true;
	}

	if(compute && !!graphicsMask)
	{
		return false;
	}

	ShaderTypeBit rtMask = ShaderTypeBit::NONE;
	if(m_rayTracingShaders.m_rayGenShader)
	{
		if(m_rayTracingShaders.m_rayGenShader->getShaderType() != ShaderType::RAY_GEN)
		{
			return false;
		}
		rtMask |= ShaderTypeBit::RAY_GEN;
	}

	for(const ShaderPtr& s : m_rayTracingShaders.m_missShaders)
	{
		if(s->getShaderType() != ShaderType::MISS)
		{
			return false;
		}
		rtMask |= ShaderTypeBit::MISS;
	}

	for(const RayTracingHitGroup& group : m_rayTracingShaders.m_hitGroups)
	{
		ShaderTypeBit localRtMask = ShaderTypeBit::NONE;
		if(group.m_anyHitShader)
		{
			if(group.m_anyHitShader->getShaderType() != ShaderType::ANY_HIT)
			{
				return false;
			}
			localRtMask |= ShaderTypeBit::ANY_HIT;
		}

		if(group.m_closestHitShader)
		{
			if(group.m_closestHitShader->getShaderType() != ShaderType::CLOSEST_HIT)
			{
				return false;
			}
			localRtMask |= ShaderTypeBit::CLOSEST_HIT;
		}

		if(!localRtMask)
		{
			return false;
		}

		rtMask |= localRtMask;
	}

	if(!!rtMask && (!!graphicsMask || compute))
	{
		return false;
	}

	if(!graphicsMask && !compute && !rtMask)
	{
		return false;
	}

	if(!!rtMask && m_rayTracingShaders.m_maxRecursionDepth == 0)
	{
		return false;
	}

	return true;
}

} // end namespace anki
