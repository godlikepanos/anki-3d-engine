// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/ShaderProgram.h>

namespace anki {

Bool ShaderProgramInitInfo::isValid() const
{
	ShaderTypeBit graphicsMask = ShaderTypeBit::kNone;
	for(ShaderType i = ShaderType::kFirstGraphics; i <= ShaderType::kLastGraphics; ++i)
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
	   && (graphicsMask & (ShaderTypeBit::kVertex | ShaderTypeBit::kFragment))
			  != (ShaderTypeBit::kVertex | ShaderTypeBit::kFragment))
	{
		return false;
	}

	Bool compute = false;
	if(m_computeShader)
	{
		if(m_computeShader->getShaderType() != ShaderType::kCompute)
		{
			return false;
		}
		compute = true;
	}

	if(compute && !!graphicsMask)
	{
		return false;
	}

	ShaderTypeBit rtMask = ShaderTypeBit::kNone;
	for(const ShaderPtr& s : m_rayTracingShaders.m_rayGenShaders)
	{
		if(s->getShaderType() != ShaderType::kRayGen)
		{
			return false;
		}
		rtMask |= ShaderTypeBit::kRayGen;
	}

	for(const ShaderPtr& s : m_rayTracingShaders.m_missShaders)
	{
		if(s->getShaderType() != ShaderType::kMiss)
		{
			return false;
		}
		rtMask |= ShaderTypeBit::kMiss;
	}

	for(const RayTracingHitGroup& group : m_rayTracingShaders.m_hitGroups)
	{
		ShaderTypeBit localRtMask = ShaderTypeBit::kNone;
		if(group.m_anyHitShader)
		{
			if(group.m_anyHitShader->getShaderType() != ShaderType::kAnyHit)
			{
				return false;
			}
			localRtMask |= ShaderTypeBit::kAnyHit;
		}

		if(group.m_closestHitShader)
		{
			if(group.m_closestHitShader->getShaderType() != ShaderType::kClosestHit)
			{
				return false;
			}
			localRtMask |= ShaderTypeBit::kClosestHit;
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
