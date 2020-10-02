// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/GrObject.h>
#include <anki/Math.h>
#include <anki/util/WeakArray.h>

namespace anki
{

/// @addtogroup graphics
/// @{

/// Specialization constant value.
class ShaderSpecializationConstValue
{
public:
	union
	{
		F32 m_float;
		I32 m_int;
		U32 m_uint;
	};

	U32 m_constantId = MAX_U32;
	ShaderVariableDataType m_dataType;

	ShaderSpecializationConstValue()
		: m_int(0)
		, m_dataType(ShaderVariableDataType::NONE)
	{
	}

	explicit ShaderSpecializationConstValue(F32 f)
		: m_float(f)
		, m_dataType(ShaderVariableDataType::F32)
	{
	}

	explicit ShaderSpecializationConstValue(I32 i)
		: m_int(i)
		, m_dataType(ShaderVariableDataType::I32)
	{
	}

	explicit ShaderSpecializationConstValue(U32 i)
		: m_int(i)
		, m_dataType(ShaderVariableDataType::U32)
	{
	}

	ShaderSpecializationConstValue(const ShaderSpecializationConstValue&) = default;

	ShaderSpecializationConstValue& operator=(const ShaderSpecializationConstValue&) = default;
};

/// Shader init info.
class ShaderInitInfo : public GrBaseInitInfo
{
public:
	ShaderType m_shaderType = ShaderType::COUNT;
	ConstWeakArray<U8> m_binary = {};

	/// @note It's OK to have entries in that array with consts that do not appear in the shader.
	ConstWeakArray<ShaderSpecializationConstValue> m_constValues;

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
};

/// GPU shader.
class Shader : public GrObject
{
	ANKI_GR_OBJECT

public:
	static const GrObjectType CLASS_TYPE = GrObjectType::SHADER;

	ShaderType getShaderType() const
	{
		ANKI_ASSERT(m_shaderType != ShaderType::COUNT);
		return m_shaderType;
	}

protected:
	ShaderType m_shaderType = ShaderType::COUNT;

	/// Construct.
	Shader(GrManager* manager, CString name)
		: GrObject(manager, CLASS_TYPE, name)
	{
	}

	/// Destroy.
	~Shader()
	{
	}

private:
	/// Allocate and initialize new instance.
	static ANKI_USE_RESULT Shader* newInstance(GrManager* manager, const ShaderInitInfo& init);
};
/// @}

} // end namespace anki
