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

/// ShaderProgram init info.
class ShaderProgramInitInfo : public GrBaseInitInfo
{
public:
	Array<ShaderPtr, U32(ShaderType::COUNT)> m_shaders = {};

	ShaderProgramInitInfo(CString name = {})
		: GrBaseInitInfo(name)
	{
	}

	ShaderProgramInitInfo(const ShaderPtr& compute, CString name = {})
		: GrBaseInitInfo(name)
	{
		m_shaders[compute->getShaderType()] = compute;
	}

	ShaderProgramInitInfo(const ShaderPtr& vert, const ShaderPtr& frag, CString name = {})
		: GrBaseInitInfo(name)
	{
		m_shaders[vert->getShaderType()] = vert;
		m_shaders[frag->getShaderType()] = frag;
	}

	Bool isValid() const
	{
		ShaderTypeBit mask = ShaderTypeBit::NONE;
		for(ShaderType i : EnumIterable<ShaderType>())
		{
			mask |= (m_shaders[i]) ? shaderTypeToBit(i) : ShaderTypeBit::NONE;
		}

		U32 invalid = 0;
		invalid |= !!(mask & ShaderTypeBit::ALL_GRAPHICS) && !!(mask & ~ShaderTypeBit::ALL_GRAPHICS);
		invalid |= !!(mask & ShaderTypeBit::COMPUTE) && !!(mask & ~ShaderTypeBit::COMPUTE);

		return invalid == 0;
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
