// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders = {};

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
		I32 invalid = 0;

		if(m_shaders[ShaderType::COMPUTE])
		{
			invalid |= m_shaders[ShaderType::VERTEX] || m_shaders[ShaderType::TESSELLATION_CONTROL]
					   || m_shaders[ShaderType::TESSELLATION_EVALUATION] || m_shaders[ShaderType::GEOMETRY]
					   || m_shaders[ShaderType::FRAGMENT];
		}
		else
		{
			invalid |= !m_shaders[ShaderType::VERTEX] || !m_shaders[ShaderType::FRAGMENT];
		}

		for(ShaderType type = ShaderType::FIRST; type < ShaderType::COUNT; ++type)
		{
			invalid |= m_shaders[type] && m_shaders[type]->getShaderType() != type;
		}

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
