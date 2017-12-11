// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
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
class ShaderProgramInitInfo : GrBaseInitInfo
{
public:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders = {};
};

/// GPU program.
class ShaderProgram : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER_PROGRAM;

protected:
	/// Construct.
	ShaderProgram(GrManager* manager)
		: GrObject(manager, CLASS_TYPE)
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
