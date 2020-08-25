// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/gr/Shader.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// @memberof RayTracingShaders
class RayTracingHitGroup
{
public:
	ShaderPtr m_closestHitShader;
	ShaderPtr m_anyHitShader;
};

/// @memberof ShaderProgramInitInfo
class RayTracingShaders
{
public:
	ShaderPtr m_rayGenShader;
	ShaderPtr m_missShader;
	WeakArray<RayTracingHitGroup> m_hitGroups;
};

/// ShaderProgram init info.
class ShaderProgramInitInfo : public GrBaseInitInfo
{
public:
	/// Option 1
	Array<ShaderPtr, U32(ShaderType::LAST_GRAPHICS + 1)> m_graphicsShaders;

	/// Option 2
	ShaderPtr m_computeShader;

	/// Option 3
	RayTracingShaders m_rayTracingShaders;

	ShaderProgramInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	Bool isValid() const
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

		if(m_rayTracingShaders.m_missShader)
		{
			if(m_rayTracingShaders.m_missShader->getShaderType() != ShaderType::MISS)
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

		return true;
	}
};

/// GPU program.
class ShaderProgram : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER_PROGRAM;

protected:
	/// Construct.
	ShaderProgram(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~ShaderProgram()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT ShaderProgram* newInstance(GrManager* manager, const ShaderProgramInitInfo& init);
};
/// @}

} // end namespace anki
