// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/Material.h>
#include <anki/resource/ResourceManager.h>
#include <anki/misc/Xml.h>

namespace anki
{

struct BuiltinVarInfo
{
	const char* m_name;
	ShaderVariableDataType m_type;
	Bool m_instanced;
};

static const Array<BuiltinVarInfo, U(BuiltinMaterialVariableId::COUNT) - 1> BUILTIN_INFOS = {
	{{"MODEL_VIEW_PROJECTION_MATRIX", ShaderVariableDataType::MAT4, true},
		{"MODEL_VIEW_MATRIX", ShaderVariableDataType::MAT4, true},
		{"VIEW_PROJECTION_MATRIX", ShaderVariableDataType::MAT4, false},
		{"VIEW_MATRIX", ShaderVariableDataType::MAT4, false},
		{"NORMAL_MATRIX", ShaderVariableDataType::MAT3, true},
		{"CAMERA_ROTATION_MATRIX", ShaderVariableDataType::MAT3, false}}};

MaterialVariable::MaterialVariable()
{
	m_mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

Material::Material(ResourceManager* manager)
	: ResourceObject(manager)
{
}

Material::~Material()
{
	m_vars.destroy(getAllocator());
	m_constValues.destroy(getAllocator());
}

Error Material::load(const ResourceFilename& filename)
{
	XmlDocument doc;
	XmlElement el;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("material", rootEl));

	// <shaderProgram>
	ANKI_CHECK(rootEl.getChildElement("shaderProgram", el));
	CString fname;
	ANKI_CHECK(el.getText(fname));
	getManager().loadResource(fname, m_prog);

	// <shadow>
	ANKI_CHECK(rootEl.getChildElementOptional("shadow", el));
	if(el)
	{
		I64 num;
		ANKI_CHECK(el.getNumber(num));
		m_shadow = num != 0;
	}

	// <levelsOfDetail>
	ANKI_CHECK(rootEl.getChildElementOptional("levelsOfDetail", el));
	if(el)
	{
		I64 num;
		ANKI_CHECK(el.getNumber(num));

		if(num <= 0 || num > I64(MAX_LODS))
		{
			ANKI_RESOURCE_LOGE("Incorrect <levelsOfDetail>: %d", num);
			return ErrorCode::USER_DATA;
		}

		m_lodCount = U8(num);
	}

	// <forwardShading>
	ANKI_CHECK(rootEl.getChildElementOptional("forwardShading", el));
	if(el)
	{
		I64 num;
		ANKI_CHECK(el.getNumber(num));
		m_forwardShading = num != 0;
	}

	// <inputs>
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		ANKI_CHECK(parseInputs(el));
	}

	return ErrorCode::NONE;
}

Error Material::parseInputs(XmlElement inputsEl)
{
	// Iterate the program's variables and get counts
	U constInputCount = 0;
	U nonConstInputCount = 0;
	for(const ShaderProgramResourceInputVariable& in : m_prog->getInputVariables())
	{
		if(in.isConstant())
		{
			++constInputCount;
		}
		else
		{
			++nonConstInputCount;
		}
	}

	m_vars.create(getAllocator(), nonConstInputCount);
	m_constValues.create(getAllocator(), constInputCount);

	// Connect the input variables
	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElementOptional("input", inputEl));
	while(inputEl)
	{
		// Get var name
		XmlElement shaderInputEl;
		ANKI_CHECK(inputEl.getChildElement("shaderInput", shaderInputEl));
		CString varName;
		ANKI_CHECK(shaderInputEl.getText(varName));

		// Try find var
		const ShaderProgramResourceInputVariable* foundVar = nullptr;
		for(const ShaderProgramResourceInputVariable& in : m_prog->getInputVariables())
		{
			if(in.getName() == varName)
			{
				foundVar = &in;
				break;
			}
		}

		if(foundVar == nullptr)
		{
			ANKI_RESOURCE_LOGE("Variable not found: %s", &varName[0]);
			return ErrorCode::USER_DATA;
		}

		// Process var
		if(foundVar->isConstant())
		{
			// Const

			XmlElement valueEl;
			ANKI_CHECK(inputEl.getChildElement("value", valueEl));

			ShaderProgramResourceConstantValue& constVal = m_constValues[--constInputCount];
			constVal.m_variable = foundVar;

			switch(foundVar->getShaderVariableDataType())
			{
			case ShaderVariableDataType::FLOAT:
				ANKI_CHECK(valueEl.getNumber(constVal.m_float));
				break;
			case ShaderVariableDataType::VEC2:
				ANKI_CHECK(valueEl.getVec2(constVal.m_vec2));
				break;
			case ShaderVariableDataType::VEC3:
				ANKI_CHECK(valueEl.getVec3(constVal.m_vec3));
				break;
			case ShaderVariableDataType::VEC4:
				ANKI_CHECK(valueEl.getVec4(constVal.m_vec4));
				break;
			default:
				ANKI_ASSERT(0);
				break;
			}
		}
		else
		{
			// Builtin or not

			XmlElement builtinEl;
			ANKI_CHECK(inputEl.getChildElementOptional("builtin", builtinEl));
			if(builtinEl)
			{
				// Builtin

				CString builtinStr;
				ANKI_CHECK(builtinEl.getText(builtinStr));

				U i;
				for(i = 0; i < BUILTIN_INFOS.getSize(); ++i)
				{
					if(builtinStr == BUILTIN_INFOS[i].m_name)
					{
						break;
					}
				}

				if(i == BUILTIN_INFOS.getSize())
				{
					ANKI_RESOURCE_LOGE("Incorrect builtin variable: %s", &builtinStr[0]);
					return ErrorCode::USER_DATA;
				}

				if(BUILTIN_INFOS[i].m_type != foundVar->getShaderVariableDataType())
				{
					ANKI_RESOURCE_LOGE(
						"The type of the builtin variable in the shader program is not the correct one: %s",
						&builtinStr[0]);
					return ErrorCode::USER_DATA;
				}

				if(foundVar->isInstanced() && !BUILTIN_INFOS[i].m_instanced)
				{
					ANKI_RESOURCE_LOGE("Builtin variable %s cannot be instanced", BUILTIN_INFOS[i].m_name);
					return ErrorCode::USER_DATA;
				}

				MaterialVariable& mtlVar = m_vars[--nonConstInputCount];
				mtlVar.m_input = foundVar;
				mtlVar.m_builtin = BuiltinMaterialVariableId(i + 1);
			}
			else
			{
				// Not built-in

				if(foundVar->isInstanced())
				{
					ANKI_RESOURCE_LOGE("Only some builtin variables can be instanced: %s", &foundVar->getName()[0]);
					return ErrorCode::USER_DATA;
				}

				MaterialVariable& mtlVar = m_vars[--nonConstInputCount];
				mtlVar.m_input = foundVar;

				XmlElement valueEl;
				ANKI_CHECK(inputEl.getChildElement("value", valueEl));

				switch(foundVar->getShaderVariableDataType())
				{
				case ShaderVariableDataType::FLOAT:
					ANKI_CHECK(valueEl.getNumber(mtlVar.m_float));
					break;
				case ShaderVariableDataType::VEC2:
					ANKI_CHECK(valueEl.getVec2(mtlVar.m_vec2));
					break;
				case ShaderVariableDataType::VEC3:
					ANKI_CHECK(valueEl.getVec3(mtlVar.m_vec3));
					break;
				case ShaderVariableDataType::VEC4:
					ANKI_CHECK(valueEl.getVec4(mtlVar.m_vec4));
					break;
				case ShaderVariableDataType::MAT3:
					ANKI_CHECK(valueEl.getMat3(mtlVar.m_mat3));
					break;
				case ShaderVariableDataType::MAT4:
					ANKI_CHECK(valueEl.getMat4(mtlVar.m_mat4));
					break;
				case ShaderVariableDataType::SAMPLER_2D:
				case ShaderVariableDataType::SAMPLER_2D_ARRAY:
				case ShaderVariableDataType::SAMPLER_3D:
				case ShaderVariableDataType::SAMPLER_CUBE:
				{
					CString texfname;
					ANKI_CHECK(valueEl.getText(texfname));
					getManager().loadResource(texfname, mtlVar.m_tex);
					break;
				}

				default:
					ANKI_ASSERT(0);
					break;
				}
			}
		}

		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	}

	if(nonConstInputCount != 0)
	{
		ANKI_RESOURCE_LOGE("Forgot to list %u shader program variables in this material", nonConstInputCount);
		return ErrorCode::USER_DATA;
	}

	if(constInputCount != 0)
	{
		ANKI_RESOURCE_LOGE("Forgot to list %u constant shader program variables in this material", constInputCount);
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

const MaterialVariant& Material::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	key.m_lod = min<U>(m_lodCount - 1, key.m_lod);

	if(!isInstanced())
	{
		ANKI_ASSERT(key.m_instanceCount == 1);
	}

	key.m_instanceCount = 1 << getInstanceGroupIdx(key.m_instanceCount);

	MaterialVariant& variant = m_variantMatrix[U(key.m_pass)][key.m_lod][getInstanceGroupIdx(key.m_instanceCount)];
	LockGuard<Mutex> lock(m_variantMatrixMtx);

	if(variant.m_variant == nullptr)
	{
		m_prog->getOrCreateVariant(key,
			WeakArray<const ShaderProgramResourceConstantValue>((m_constValues.getSize()) ? &m_constValues[0] : nullptr,
									   m_constValues.getSize()),
			variant.m_variant);
	}

	return variant;
}

U Material::getInstanceGroupIdx(U instanceCount)
{
	ANKI_ASSERT(instanceCount > 0);
	instanceCount = nextPowerOfTwo(instanceCount);
	ANKI_ASSERT(instanceCount <= MAX_INSTANCES);
	return log2(instanceCount);
}

} // end namespace anki
