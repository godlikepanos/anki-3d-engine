// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Gr/GrObject.h>
#include <AnKi/Util/WeakArray.h>

namespace anki {

// Shader init info.
class ShaderInitInfo : public GrBaseInitInfo
{
public:
	ShaderType m_shaderType = ShaderType::kCount;
	ConstWeakArray<U8> m_binary;

	ShaderReflection m_reflection;

	ShaderInitInfo()
	{
	}

	ShaderInitInfo(CString name)
		: GrBaseInitInfo(name)
	{
	}

	ShaderInitInfo(ShaderType type, ConstWeakArray<U8> bin, CString name = {})
		: GrBaseInitInfo(name)
		, m_shaderType(type)
		, m_binary(bin)
	{
	}

	void validate() const
	{
		ANKI_ASSERT(m_shaderType != ShaderType::kCount);
		ANKI_ASSERT(m_binary.getSize() > 0);
		m_reflection.validate();
	}
};

// GPU shader.
class Shader : public GrObject
{
	ANKI_GR_OBJECT

public:
	static constexpr GrObjectType kClassType = GrObjectType::kShader;

	ShaderType getShaderType() const
	{
		ANKI_ASSERT(m_shaderType != ShaderType::kCount);
		return m_shaderType;
	}

	U32 getShaderBinarySize() const
	{
		ANKI_ASSERT(m_shaderBinarySize);
		return m_shaderBinarySize;
	}

	// Pixel shader had a discard.
	U32 hasDiscard() const
	{
		ANKI_ASSERT(m_shaderType == ShaderType::kPixel);
		return m_hasDiscard;
	}

protected:
	U32 m_shaderBinarySize = 0;

	ShaderType m_shaderType = ShaderType::kCount;

	Bool m_hasDiscard = false;

	Shader(CString name)
		: GrObject(kClassType, name)
	{
	}

	~Shader()
	{
	}

private:
	[[nodiscard]] static Shader* newInstance(const ShaderInitInfo& init);
};

} // end namespace anki
