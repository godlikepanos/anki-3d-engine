// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResource.h>
#include <anki/misc/Xml.h>
#include <anki/resource/ShaderLoader.h>
#include <anki/resource/RenderingKey.h>

namespace anki
{

static ANKI_USE_RESULT Error computeShaderVariableDataType(const CString& str, ShaderVariableDataType& out)
{
	Error err = ErrorCode::NONE;

	if(str == "float")
	{
		out = ShaderVariableDataType::FLOAT;
	}
	else if(str == "vec2")
	{
		out = ShaderVariableDataType::VEC2;
	}
	else if(str == "vec3")
	{
		out = ShaderVariableDataType::VEC3;
	}
	else if(str == "vec4")
	{
		out = ShaderVariableDataType::VEC4;
	}
	else if(str == "mat3")
	{
		out = ShaderVariableDataType::MAT3;
	}
	else if(str == "mat4")
	{
		out = ShaderVariableDataType::MAT4;
	}
	else if(str == "sampler2D")
	{
		out = ShaderVariableDataType::SAMPLER_2D;
	}
	else if(str == "sampler2DArray")
	{
		out = ShaderVariableDataType::SAMPLER_2D_ARRAY;
	}
	else if(str == "samplerCube")
	{
		out = ShaderVariableDataType::SAMPLER_CUBE;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Incorrect variable type %s", &str[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

static CString toString(ShaderVariableDataType in)
{
	CString out;

	switch(in)
	{
	case ShaderVariableDataType::FLOAT:
		out = "float";
		break;
	case ShaderVariableDataType::VEC2:
		out = "vec2";
		break;
	case ShaderVariableDataType::VEC3:
		out = "vec3";
		break;
	case ShaderVariableDataType::VEC4:
		out = "vec4";
		break;
	case ShaderVariableDataType::MAT4:
		out = "mat4";
		break;
	case ShaderVariableDataType::MAT3:
		out = "mat3";
		break;
	case ShaderVariableDataType::SAMPLER_2D:
		out = "sampler2D";
		break;
	case ShaderVariableDataType::SAMPLER_3D:
		out = "sampler3D";
		break;
	case ShaderVariableDataType::SAMPLER_2D_ARRAY:
		out = "sampler2DArray";
		break;
	case ShaderVariableDataType::SAMPLER_CUBE:
		out = "samplerCube";
		break;
	default:
		ANKI_ASSERT(0);
	};

	return out;
}

/// Given a string return info about the shader.
static ANKI_USE_RESULT Error getShaderInfo(const CString& str, ShaderType& type, ShaderTypeBit& bit, U& idx)
{
	Error err = ErrorCode::NONE;

	if(str == "vert")
	{
		type = ShaderType::VERTEX;
		bit = ShaderTypeBit::VERTEX;
		idx = 0;
	}
	else if(str == "tesc")
	{
		type = ShaderType::TESSELLATION_CONTROL;
		bit = ShaderTypeBit::TESSELLATION_CONTROL;
		idx = 1;
	}
	else if(str == "tese")
	{
		type = ShaderType::TESSELLATION_EVALUATION;
		bit = ShaderTypeBit::TESSELLATION_EVALUATION;
		idx = 2;
	}
	else if(str == "geom")
	{
		type = ShaderType::GEOMETRY;
		bit = ShaderTypeBit::GEOMETRY;
		idx = 3;
	}
	else if(str == "frag")
	{
		type = ShaderType::FRAGMENT;
		bit = ShaderTypeBit::FRAGMENT;
		idx = 4;
	}
	else
	{
		ANKI_RESOURCE_LOGE("Incorrect type %s", &str[0]);
		err = ErrorCode::USER_DATA;
	}

	return err;
}

ShaderProgramResource::ShaderProgramResource(ResourceManager* manager)
	: ResourceObject(manager)
{
}

ShaderProgramResource::~ShaderProgramResource()
{
	auto alloc = getAllocator();

	while(!m_variants.isEmpty())
	{
		ShaderProgramResourceVariant* variant = &(*m_variants.getBegin());
		m_variants.erase(variant);

		variant->m_blockInfos.destroy(alloc);
		variant->m_texUnits.destroy(alloc);
		alloc.deleteInstance(variant);
	}

	m_inputVars.destroy(alloc);
	m_inputVarsNames.destroy(alloc);

	for(String& s : m_sources)
	{
		s.destroy(alloc);
	}
}

Error ShaderProgramResource::load(const ResourceFilename& filename)
{
	XmlElement el;
	XmlDocument doc;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <shaderProgram>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("shaderProgram", rootEl));

	// <shaders>
	XmlElement shadersEl;
	ANKI_CHECK(rootEl.getChildElement("shaders", shadersEl));

	// <shader>
	// - Count the input variables and the length of their name
	// - Create the constant code
	U inputVarCount = 0;
	U inputsNameLen = 0;
	XmlElement shaderEl;
	StringAuto constantSrc(getTempAllocator());
	ANKI_CHECK(shadersEl.getChildElement("shader", shaderEl));
	do
	{
		XmlElement inputsEl;
		ANKI_CHECK(shaderEl.getChildElementOptional("inputs", inputsEl));

		if(inputsEl)
		{
			XmlElement inputEl;
			ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
			do
			{
				// <name>
				XmlElement nameEl;
				ANKI_CHECK(inputEl.getChildElement("name", nameEl));
				CString name;
				ANKI_CHECK(nameEl.getText(name));
				inputsNameLen += name.getLength() + 1;

				// <type>
				ANKI_CHECK(inputEl.getChildElement("type", el));
				CString typeTxt;
				ANKI_CHECK(el.getText(typeTxt));

				// <depth>
				Bool depth = true;
				ANKI_CHECK(inputEl.getChildElementOptional("depth", el));
				if(el)
				{
					I64 num;
					ANKI_CHECK(el.getI64(num));
					depth = num != 0;
				}

				// <const>
				ANKI_CHECK(inputEl.getChildElementOptional("const", el));
				if(el)
				{
					I64 num;
					ANKI_CHECK(el.getI64(num));
					if(num)
					{
						StringAuto str(getTempAllocator());
						str.sprintf("#if !(DEPTH && %d)\n"
									"const %s %s = %s(%s_const);\n"
									"#endif\n",
							!depth,
							&typeTxt[0],
							&name[0],
							&typeTxt[0],
							&name[0]);
						constantSrc.append(str);
					}
				}

				++inputVarCount;

				ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
			} while(inputEl);
		}

		ANKI_CHECK(shaderEl.getNextSiblingElement("shader", shaderEl));
	} while(shaderEl);

	if(inputVarCount)
	{
		m_inputVars.create(getAllocator(), inputVarCount);
		m_inputVarsNames.create(getAllocator(), inputsNameLen);
	}

	// <shader> again
	ShaderTypeBit presentShaders = ShaderTypeBit::NONE;
	ANKI_CHECK(shadersEl.getChildElement("shader", shaderEl));
	do
	{
		// <type>
		ANKI_CHECK(shaderEl.getChildElement("type", el));
		CString shaderTypeTxt;
		ANKI_CHECK(el.getText(shaderTypeTxt));
		ShaderType shaderType;
		ShaderTypeBit shaderTypeBit;
		U shaderIdx;
		ANKI_CHECK(getShaderInfo(shaderTypeTxt, shaderType, shaderTypeBit, shaderIdx));

		if(!!(presentShaders & shaderTypeBit))
		{
			ANKI_RESOURCE_LOGE("Shader is present more than once: %s", &shaderTypeTxt[0]);
			return ErrorCode::USER_DATA;
		}
		presentShaders |= shaderTypeBit;

		if(shaderType == ShaderType::TESSELLATION_CONTROL || shaderType == ShaderType::TESSELLATION_EVALUATION)
		{
			m_tessellation = true;
		}

		// <inputs>
		XmlElement inputsEl;
		ANKI_CHECK(shaderEl.getChildElementOptional("inputs", inputsEl));

		if(inputsEl)
		{
			ANKI_CHECK(parseInputs(inputsEl));
		}

		// <source>
		ANKI_CHECK(shaderEl.getChildElement("source", el));
		CString shaderSource;
		ANKI_CHECK(el.getText(shaderSource));

		ShaderLoader loader(&getManager());
		ANKI_CHECK(loader.parseSourceString(shaderSource));

		if(constantSrc)
		{
			m_sources[shaderIdx].append(getAllocator(), constantSrc);
		}

		m_sources[shaderIdx].append(getAllocator(), loader.getShaderSource());

		// Advance
		ANKI_CHECK(shaderEl.getNextSiblingElement("shader", shaderEl));
	} while(shaderEl);

	// Sanity checks
	if(!(presentShaders & ShaderTypeBit::VERTEX))
	{
		ANKI_RESOURCE_LOGE("Missing vertex shader");
		return ErrorCode::USER_DATA;
	}

	if(!(presentShaders & ShaderTypeBit::FRAGMENT))
	{
		ANKI_RESOURCE_LOGE("Missing fragment shader");
		return ErrorCode::USER_DATA;
	}

	return ErrorCode::NONE;
}

Error ShaderProgramResource::parseInputs(XmlElement& inputsEl)
{
	U inputVarCount = 0;
	U namePos = 0;

	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
	do
	{
		XmlElement el;
		I64 num;

		ShaderProgramResourceInputVariable& var = m_inputVars[inputVarCount];
		var.m_idx = inputVarCount;

		// <name>
		ANKI_CHECK(inputEl.getChildElement("name", el));
		CString name;
		ANKI_CHECK(el.getText(name));
		memcpy(&m_inputVarsNames[namePos], &name[0], name.getLength() + 1);
		var.m_name = &m_inputVarsNames[namePos];
		namePos += name.getLength() + 1;

		// <type>
		ANKI_CHECK(inputEl.getChildElement("type", el));
		CString type;
		ANKI_CHECK(el.getText(type));
		ANKI_CHECK(computeShaderVariableDataType(type, var.m_dataType));

		// <depth>
		ANKI_CHECK(inputEl.getChildElementOptional("depth", el));
		if(el)
		{
			ANKI_CHECK(el.getI64(num));
			var.m_depth = num != 0;
		}
		else
		{
			var.m_depth = true;
		}

		// <const>
		ANKI_CHECK(inputEl.getChildElementOptional("const", el));
		if(el)
		{
			ANKI_CHECK(el.getI64(num));
			var.m_const = num != 0;
		}
		else
		{
			var.m_const = false;
		}

		// <instanced>
		ANKI_CHECK(inputEl.getChildElementOptional("instanced", el));
		if(el)
		{
			ANKI_CHECK(el.getI64(num));
			var.m_instanced = num != 0;

			if(var.m_instanced)
			{
				m_instanced = true;
			}
		}
		else
		{
			var.m_instanced = false;
		}

		// Sanity checks
		if(var.isTexture() && var.m_const)
		{
			ANKI_RESOURCE_LOGE("Can't have a texture to be const: %s", &var.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		for(U i = 0; i < inputVarCount; ++i)
		{
			const ShaderProgramResourceInputVariable& b = m_inputVars[i];
			if(b.m_name == var.m_name)
			{
				ANKI_RESOURCE_LOGE("Duplicate input variable found: %s", &var.m_name[0]);
				return ErrorCode::USER_DATA;
			}
		}

		if(var.m_dataType >= ShaderVariableDataType::MATRIX_FIRST
			&& var.m_dataType <= ShaderVariableDataType::MATRIX_LAST
			&& var.m_const)
		{
			ANKI_RESOURCE_LOGE("Matrix input variable cannot be constant: %s", &var.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		++inputVarCount;

		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	ANKI_ASSERT(namePos == m_inputVarsNames.getSize());

	return ErrorCode::NONE;
}

U64 ShaderProgramResource::computeVariantHash(
	const RenderingKey& key, WeakArray<ShaderProgramResourceConstantValue> constants) const
{
	static_assert(isPacked<RenderingKey>(), "See file");
	U hash = computeHash(&key, sizeof(key));

	if(constants.getSize())
	{
		hash = appendHash(&constants[0], sizeof(constants[0]) * constants.getSize(), hash);
	}

	return hash;
}

Error ShaderProgramResource::getOrCreateVariant(const RenderingKey& key,
	WeakArray<ShaderProgramResourceConstantValue> constants,
	const ShaderProgramResourceVariant*& variant)
{
	U64 hash = computeVariantHash(key, constants);

	LockGuard<Mutex> lock(m_mtx);

	auto it = m_variants.find(hash);
	if(it != m_variants.getEnd())
	{
		variant = &(*it);
	}
	else
	{
		// Create one
		ShaderProgramResourceVariant* v = getAllocator().newInstance<ShaderProgramResourceVariant>();
		Error err = initVariant(key, constants, *v);
		if(err)
		{
			getAllocator().deleteInstance(v);
			return err;
		}

		m_variants.pushBack(hash, v);
		variant = v;
	}

	return ErrorCode::NONE;
}

Error ShaderProgramResource::initVariant(const RenderingKey& key,
	WeakArray<ShaderProgramResourceConstantValue> constants,
	ShaderProgramResourceVariant& variant)
{
	U instanceCount = key.m_instanceCount;
	Bool tessellation = key.m_tessellation;
	U lod = key.m_lod;
	Pass pass = key.m_pass;

	// Preconditions
	if(tessellation)
	{
		ANKI_ASSERT(m_tessellation);
	}

	if(instanceCount > 1)
	{
		ANKI_ASSERT(m_instanced);
	}

	variant.m_activeInputVars.unsetAll();
	variant.m_blockInfos.create(getAllocator(), m_inputVars.getSize());
	variant.m_texUnits.create(getAllocator(), m_inputVars.getSize());
	if(m_inputVars)
	{
		memorySet<I16>(&variant.m_texUnits[0], -1, variant.m_texUnits.getSize());
	}

	// - Compute the block info for each var
	// - Activate vars
	// - Compute varius strings
	StringListAuto blockCode(getTempAllocator());
	StringListAuto constsCode(getTempAllocator());
	StringListAuto texturesCode(getTempAllocator());
	U texUnit = 0;

	for(const ShaderProgramResourceInputVariable& in : m_inputVars)
	{
		if(pass == Pass::SM && !in.m_depth)
		{
			continue;
		}

		variant.m_activeInputVars.set(in.m_idx);

		// Init block info
		if(in.inBlock())
		{
			ShaderVariableBlockInfo& blockInfo = variant.m_blockInfos[in.m_idx];

			// std140 rules
			blockInfo.m_offset = variant.m_uniBlockSize;
			blockInfo.m_arraySize = in.m_instanced ? instanceCount : 1;

			if(in.m_dataType == ShaderVariableDataType::FLOAT)
			{
				blockInfo.m_arrayStride = sizeof(Vec4);

				if(blockInfo.m_arraySize == 1)
				{
					// No need to align the in.m_offset
					variant.m_uniBlockSize += sizeof(F32);
				}
				else
				{
					alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
					variant.m_uniBlockSize += sizeof(Vec4) * blockInfo.m_arraySize;
				}
			}
			else if(in.m_dataType == ShaderVariableDataType::VEC2)
			{
				blockInfo.m_arrayStride = sizeof(Vec4);

				if(blockInfo.m_arraySize == 1)
				{
					alignRoundUp(sizeof(Vec2), blockInfo.m_offset);
					variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec2);
				}
				else
				{
					alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
					variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
				}
			}
			else if(in.m_dataType == ShaderVariableDataType::VEC3)
			{
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				blockInfo.m_arrayStride = sizeof(Vec4);

				if(blockInfo.m_arraySize == 1)
				{
					variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec3);
				}
				else
				{
					variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
				}
			}
			else if(in.m_dataType == ShaderVariableDataType::VEC4)
			{
				blockInfo.m_arrayStride = sizeof(Vec4);
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * blockInfo.m_arraySize;
			}
			else if(in.m_dataType == ShaderVariableDataType::MAT3)
			{
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				blockInfo.m_arrayStride = sizeof(Vec4) * 3;
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Vec4) * 3 * blockInfo.m_arraySize;
				blockInfo.m_matrixStride = sizeof(Vec4);
			}
			else if(in.m_dataType == ShaderVariableDataType::MAT4)
			{
				alignRoundUp(sizeof(Vec4), blockInfo.m_offset);
				blockInfo.m_arrayStride = sizeof(Mat4);
				variant.m_uniBlockSize = blockInfo.m_offset + sizeof(Mat4) * blockInfo.m_arraySize;
				blockInfo.m_matrixStride = sizeof(Vec4);
			}
			else
			{
				ANKI_ASSERT(0);
			}

			blockCode.pushBackSprintf("%s %s;\n", &toString(in.m_dataType)[0], &in.m_name[0]);
		} // if(in.inBlock())

		if(in.m_const)
		{
			// Find the const value
			const ShaderProgramResourceConstantValue* constVal = nullptr;
			for(const ShaderProgramResourceConstantValue& c : constants)
			{
				if(c.m_variable == &in)
				{
					constVal = &c;
					break;
				}
			}

			ANKI_ASSERT(constVal);

			switch(in.m_dataType)
			{
			case ShaderVariableDataType::FLOAT:
				constsCode.pushBackSprintf("#define %s_const %f\n", &in.m_name[0], constVal->m_float);
				break;
			case ShaderVariableDataType::VEC2:
				constsCode.pushBackSprintf(
					"#define %s_const %f, %f\n", &in.m_name[0], constVal->m_vec2.x(), constVal->m_vec2.y());
				break;
			case ShaderVariableDataType::VEC3:
				constsCode.pushBackSprintf("#define %s_const %f, %f, %f\n",
					&in.m_name[0],
					constVal->m_vec3.x(),
					constVal->m_vec3.y(),
					constVal->m_vec3.z());
				break;
			case ShaderVariableDataType::VEC4:
				constsCode.pushBackSprintf("#define %s_const %f, %f, %f, %f\n",
					&in.m_name[0],
					constVal->m_vec4.x(),
					constVal->m_vec4.y(),
					constVal->m_vec4.z(),
					constVal->m_vec4.w());
				break;
			default:
				ANKI_ASSERT(0);
			}
		}

		if(in.isTexture())
		{
			texturesCode.pushBackSprintf("layout(ANKI_TEX_BINDING(0, %u)) uniform %s %s;\n",
				texUnit,
				&toString(in.m_dataType)[0],
				&in.m_name[0]);

			variant.m_texUnits[in.m_idx] = texUnit;
			++texUnit;
		}
	}

	// Write the source header
	StringListAuto shaderHeaderSrc(getTempAllocator());
	shaderHeaderSrc.pushBackSprintf("#define INSTANCE_COUNT %u\n"
									"#define TESSELATION %u\n"
									"#define LOD %u\n"
									"#define COLOR %u\n"
									"#define DEPTH %u\n",
		instanceCount,
		tessellation,
		lod,
		pass == Pass::MS_FS,
		pass == Pass::SM);

	if(constsCode)
	{
		StringAuto str(getTempAllocator());
		constsCode.join("", str);
		shaderHeaderSrc.pushBack(str.toCString());
	}

	if(variant.m_uniBlockSize)
	{
		StringAuto str(getTempAllocator());
		blockCode.join("", str);

		shaderHeaderSrc.pushBack("layout(ANKI_UBO_BINDING(0, 0)) uniform u0_ {\n");
		shaderHeaderSrc.pushBack(str.toCString());
		shaderHeaderSrc.pushBack("};\n");
	}

	if(texturesCode)
	{
		StringAuto str(getTempAllocator());
		texturesCode.join("", str);
		shaderHeaderSrc.pushBack(str.toCString());
	}

	StringAuto shaderHeader(getTempAllocator());
	shaderHeaderSrc.join("", shaderHeader);

	// Create the shaders and the program
	Array<ShaderPtr, 5> shaders;
	for(ShaderType i = ShaderType::FIRST_GRAPHICS; i <= ShaderType::LAST_GRAPHICS; ++i)
	{
		if(!tessellation && (i == ShaderType::TESSELLATION_CONTROL || i == ShaderType::TESSELLATION_EVALUATION))
		{
			continue;
		}

		if(i == ShaderType::GEOMETRY)
		{
			continue;
		}

		StringAuto src(getTempAllocator());
		src.append(shaderHeader);
		src.append(m_sources[i]);

		shaders[i] = getManager().getGrManager().newInstance<Shader>(i, src.toCString());
	}

	if(tessellation)
	{
		variant.m_prog = getManager().getGrManager().newInstance<ShaderProgram>(shaders[ShaderType::VERTEX],
			shaders[ShaderType::TESSELLATION_CONTROL],
			shaders[ShaderType::TESSELLATION_EVALUATION],
			shaders[ShaderType::GEOMETRY],
			shaders[ShaderType::FRAGMENT]);
	}
	else
	{
		variant.m_prog = getManager().getGrManager().newInstance<ShaderProgram>(
			shaders[ShaderType::VERTEX], shaders[ShaderType::FRAGMENT]);
	}

	return ErrorCode::NONE;
}

} // end namespace anki