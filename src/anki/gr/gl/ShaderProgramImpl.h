// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/ShaderProgram.h>
#include <anki/gr/gl/GlObject.h>

namespace anki
{

/// @addtogroup opengl
/// @{

class ShaderProgramImplReflection
{
public:
	struct Uniform
	{
		I32 m_location;
		U32 m_pushConstantOffset;
		ShaderVariableDataType m_type;
		U8 m_arrSize;
	};

	DynamicArray<Uniform> m_uniforms;
	U32 m_uniformDataSize = 0;
};

/// Shader program implementation.
class ShaderProgramImpl final : public ShaderProgram, public GlObject
{
public:
	ShaderProgramImpl(GrManager* manager, CString name)
		: ShaderProgram(manager, name)
	{
	}

	~ShaderProgramImpl();

	ANKI_USE_RESULT Error initGraphics(
		ShaderPtr vert, ShaderPtr tessc, ShaderPtr tesse, ShaderPtr geom, ShaderPtr frag);
	ANKI_USE_RESULT Error initCompute(ShaderPtr comp);

	// Do that only when is needed to avoid serializing the thread the driver is using for compilation.
	const ShaderProgramImplReflection& getReflection();

private:
	Array<ShaderPtr, U(ShaderType::COUNT)> m_shaders;
	ShaderProgramImplReflection m_refl;
	Bool8 m_reflInitialized = false;

	ANKI_USE_RESULT Error link(GLuint vert, GLuint frag);
};
/// @}

} // end namespace anki
