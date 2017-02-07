// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/MaterialLoader.h>
#include <anki/util/Assert.h>
#include <anki/misc/Xml.h>
#include <anki/util/Logger.h>
#include <anki/util/Functions.h>
#include <algorithm>

namespace anki
{

/// Given a string return info about the shader
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

static ANKI_USE_RESULT Error computeBuiltin(const CString& name, BuiltinMaterialVariableId& out)
{
	if(name == "anki_mvp")
	{
		out = BuiltinMaterialVariableId::MVP_MATRIX;
	}
	else if(name == "anki_mv")
	{
		out = BuiltinMaterialVariableId::MV_MATRIX;
	}
	else if(name == "anki_vp")
	{
		out = BuiltinMaterialVariableId::VP_MATRIX;
	}
	else if(name == "anki_n")
	{
		out = BuiltinMaterialVariableId::NORMAL_MATRIX;
	}
	else if(name == "anki_billboardMvp")
	{
		out = BuiltinMaterialVariableId::BILLBOARD_MVP_MATRIX;
	}
	else if(name == "anki_tessLevel")
	{
		out = BuiltinMaterialVariableId::MAX_TESS_LEVEL;
	}
	else if(name == "anki_msDepthRt")
	{
		out = BuiltinMaterialVariableId::MS_DEPTH_MAP;
	}
	else if(name.find("anki_") == 0)
	{
		ANKI_RESOURCE_LOGE("Unknown builtin %s", &name[0]);
		return ErrorCode::USER_DATA;
	}
	else
	{
		out = BuiltinMaterialVariableId::NONE;
	}

	return ErrorCode::NONE;
}

static ShaderVariableDataType getBuiltinType(BuiltinMaterialVariableId in)
{
	ShaderVariableDataType out = ShaderVariableDataType::SAMPLER_2D;

	switch(in)
	{
	case BuiltinMaterialVariableId::MVP_MATRIX:
		out = ShaderVariableDataType::MAT4;
		break;
	case BuiltinMaterialVariableId::MV_MATRIX:
		out = ShaderVariableDataType::MAT4;
		break;
	case BuiltinMaterialVariableId::VP_MATRIX:
		out = ShaderVariableDataType::MAT4;
		break;
	case BuiltinMaterialVariableId::NORMAL_MATRIX:
		out = ShaderVariableDataType::MAT3;
		break;
	case BuiltinMaterialVariableId::BILLBOARD_MVP_MATRIX:
		out = ShaderVariableDataType::MAT4;
		break;
	case BuiltinMaterialVariableId::MAX_TESS_LEVEL:
		out = ShaderVariableDataType::FLOAT;
		break;
	case BuiltinMaterialVariableId::MS_DEPTH_MAP:
		out = ShaderVariableDataType::SAMPLER_2D;
		break;
	default:
		ANKI_ASSERT(0);
		break;
	}

	return out;
}

void MaterialLoaderInputVariable::move(MaterialLoaderInputVariable& b)
{
	m_type = b.m_type;
	m_builtin = b.m_builtin;
	m_flags = b.m_flags;

	m_binding = b.m_binding;
	m_index = b.m_index;

	m_shaderDefinedMask = b.m_shaderDefinedMask;
	m_shaderReferencedMask = b.m_shaderReferencedMask;
}

CString MaterialLoaderInputVariable::typeStr() const
{
	return toString(m_type);
}

MaterialLoader::MaterialLoader(GenericMemoryPoolAllocator<U8> alloc)
	: m_alloc(alloc)
	, m_source{{{alloc}, {alloc}, {alloc}, {alloc}, {alloc}}}
	, m_sourceBaked{{{alloc}, {alloc}, {alloc}, {alloc}, {alloc}}}
	, m_inputs{alloc}
{
}

MaterialLoader::~MaterialLoader()
{
}

Error MaterialLoader::parseXmlDocument(const XmlDocument& doc)
{
	XmlElement materialEl, el;

	// <levelsOfDetail>
	ANKI_CHECK(doc.getChildElement("material", materialEl));

	// <levelsOfDetail>
	ANKI_CHECK(materialEl.getChildElementOptional("levelsOfDetail", el));
	if(el)
	{
		I64 tmp;
		ANKI_CHECK(el.getI64(tmp));
		m_lodCount = (tmp < 1) ? 1 : tmp;
	}
	else
	{
		m_lodCount = 1;
	}

	if(m_lodCount > MAX_LODS)
	{
		ANKI_RESOURCE_LOGW("Too many <levelsOfDetail>");
		m_lodCount = MAX_LODS;
	}

	// <shadow>
	ANKI_CHECK(materialEl.getChildElementOptional("shadow", el));
	if(el)
	{
		I64 tmp;
		ANKI_CHECK(el.getI64(tmp));
		m_shadow = tmp;
	}

	// <forwardShading>
	ANKI_CHECK(materialEl.getChildElementOptional("forwardShading", el));
	if(el)
	{
		I64 tmp;
		ANKI_CHECK(el.getI64(tmp));
		m_forwardShading = tmp;
	}

	// <programs>
	ANKI_CHECK(materialEl.getChildElement("programs", el));
	ANKI_CHECK(parseProgramsTag(el));

	return ErrorCode::NONE;
}

Error MaterialLoader::parseProgramsTag(const XmlElement& el)
{
	//
	// First gather all the inputs
	//
	XmlElement programEl;
	ANKI_CHECK(el.getChildElement("program", programEl));
	do
	{
		ANKI_CHECK(parseInputsTag(programEl));

		ANKI_CHECK(programEl.getNextSiblingElement("program", programEl));
	} while(programEl);

	processInputs();

	//
	// Then parse the includes, operations and other parts of the program
	//
	ANKI_CHECK(el.getChildElement("program", programEl));
	do
	{
		ANKI_CHECK(parseProgramTag(programEl));

		ANKI_CHECK(programEl.getNextSiblingElement("program", programEl));
	} while(programEl);

	//
	// Sanity checks
	//

	// Check that all inputs are referenced
	for(auto& in : m_inputs)
	{
		if(in.m_shaderDefinedMask != in.m_shaderReferencedMask)
		{
			ANKI_RESOURCE_LOGE("Variable not referenced or not defined %s", &in.m_name[0]);
			return ErrorCode::USER_DATA;
		}
	}

	return ErrorCode::NONE;
}

Error MaterialLoader::parseProgramTag(const XmlElement& programEl)
{
	XmlElement el;

	// <type>
	ANKI_CHECK(programEl.getChildElement("type", el));
	CString type;
	ANKI_CHECK(el.getText(type));
	ShaderTypeBit glshaderbit;
	ShaderType glshader;
	U shaderidx;
	ANKI_CHECK(getShaderInfo(type, glshader, glshaderbit, shaderidx));

	auto& lines = m_source[shaderidx];

	if(glshader == ShaderType::TESSELLATION_CONTROL || glshader == ShaderType::TESSELLATION_EVALUATION)
	{
		m_tessellation = true;
	}

	// Some instancing crap
	lines.pushBackSprintf("\n#if INSTANCE_COUNT > 1\n"
						  "#define INSTANCE_ID [%s]\n"
						  "#define INSTANCED [INSTANCE_COUNT]\n"
						  "#else\n"
						  "#define INSTANCE_ID\n"
						  "#define INSTANCED\n"
						  "#endif\n",
		glshader == ShaderType::VERTEX ? "gl_InstanceID" : "in_instanceId");

	// <includes></includes>
	XmlElement includesEl;
	ANKI_CHECK(programEl.getChildElement("includes", includesEl));
	XmlElement includeEl;
	ANKI_CHECK(includesEl.getChildElement("include", includeEl));

	do
	{
		CString tmp;
		ANKI_CHECK(includeEl.getText(tmp));
		lines.pushBackSprintf("#include \"%s\"", &tmp[0]);

		ANKI_CHECK(includeEl.getNextSiblingElement("include", includeEl));
	} while(includeEl);

	// Inputs

	// Block
	if((m_uniformBlockReferencedMask & glshaderbit) != ShaderTypeBit::NONE)
	{
		lines.pushBack("\nlayout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform u00_\n{");

		for(Input& in : m_inputs)
		{
			if(in.m_flags.m_inBlock)
			{
				lines.pushBack(&in.m_line[0]);
			}
		}

		lines.pushBack("};");
	}

	// Other variables
	for(Input& in : m_inputs)
	{
		if(!in.m_flags.m_inBlock && (in.m_shaderDefinedMask & glshaderbit) != ShaderTypeBit::NONE
			&& !in.m_flags.m_specialBuiltin)
		{
			lines.pushBack(&in.m_line[0]);
		}
	}

	// <operations></operations>
	lines.pushBack("\nvoid main()\n{");

	if(!m_forwardShading && glshader == ShaderType::FRAGMENT)
	{
		// Write default values
		lines.pushBack("#if defined(writeRts_DEFINED)");
		lines.pushBack("writeRts(vec3(0.0), vec3(1.0), vec3(0.04), 0.5, 0.0, 0.0, 0.0);");
		lines.pushBack("#endif");
	}

	XmlElement opsEl;
	ANKI_CHECK(programEl.getChildElement("operations", opsEl));
	XmlElement opEl;
	ANKI_CHECK(opsEl.getChildElement("operation", opEl));
	do
	{
		String out;
		ANKI_CHECK(parseOperationTag(opEl, glshader, glshaderbit, out));
		lines.pushBack(&out[0]);
		out.destroy(m_alloc);

		// Advance
		ANKI_CHECK(opEl.getNextSiblingElement("operation", opEl));
	} while(opEl);

	lines.pushBack("}\n");
	return ErrorCode::NONE;
}

Error MaterialLoader::parseInputsTag(const XmlElement& programEl)
{
	XmlElement el;
	CString cstr;
	XmlElement inputsEl;
	ANKI_CHECK(programEl.getChildElementOptional("inputs", inputsEl));
	if(!inputsEl)
	{
		return ErrorCode::NONE;
	}

	// Get shader type
	ShaderTypeBit glshaderbit;
	ShaderType glshader;
	U shaderidx;
	ANKI_CHECK(programEl.getChildElement("type", el));
	ANKI_CHECK(el.getText(cstr));
	ANKI_CHECK(getShaderInfo(cstr, glshader, glshaderbit, shaderidx));

	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
	do
	{
		Input in(m_alloc);

		// <name>
		ANKI_CHECK(inputEl.getChildElement("name", el));
		ANKI_CHECK(el.getText(cstr));
		in.m_name.create(cstr);

		ANKI_CHECK(computeBuiltin(in.m_name.toCString(), in.m_builtin));

		if(in.m_builtin != BuiltinMaterialVariableId::NONE)
		{
			in.m_flags.m_builtin = true;
			if(in.m_builtin == BuiltinMaterialVariableId::MS_DEPTH_MAP)
			{
				in.m_flags.m_specialBuiltin = true;
			}
		}

		// Check if instanced
		if(in.m_builtin == BuiltinMaterialVariableId::MVP_MATRIX || in.m_builtin == BuiltinMaterialVariableId::MV_MATRIX
			|| in.m_builtin == BuiltinMaterialVariableId::NORMAL_MATRIX
			|| in.m_builtin == BuiltinMaterialVariableId::BILLBOARD_MVP_MATRIX)
		{
			in.m_flags.m_instanced = true;
			m_instanced = true;
		}

		// <type>
		ANKI_CHECK(inputEl.getChildElement("type", el));
		ANKI_CHECK(el.getText(cstr));
		ANKI_CHECK(computeShaderVariableDataType(cstr, in.m_type));

		if(in.m_flags.m_builtin)
		{
			if(getBuiltinType(in.m_builtin) != in.m_type)
			{
				ANKI_RESOURCE_LOGE("Builtin variable %s cannot be of type %s", &in.m_name[0], &cstr[0]);
				return ErrorCode::USER_DATA;
			}
		}

		// <const>
		ANKI_CHECK(inputEl.getChildElementOptional("const", el));
		if(el)
		{
			I64 tmp;
			ANKI_CHECK(el.getI64(tmp));
			in.m_flags.m_const = tmp;
		}

		if(in.m_flags.m_builtin && in.m_flags.m_const)
		{
			ANKI_RESOURCE_LOGE("Builtins cannot be consts: %s", &in.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		// <value>
		ANKI_CHECK(inputEl.getChildElementOptional("value", el));
		if(el)
		{
			ANKI_CHECK(el.getText(cstr));

			if(!cstr)
			{
				ANKI_RESOURCE_LOGE("Value tag is empty for: %s", &in.m_name[0]);
				return ErrorCode::USER_DATA;
			}

			if(in.m_flags.m_builtin)
			{
				ANKI_RESOURCE_LOGE("Builtins cannot have value: %s", &in.m_name[0]);
				return ErrorCode::USER_DATA;
			}

			in.m_value.splitString(cstr, ' ');
		}
		else
		{
			if(!in.m_flags.m_builtin)
			{
				ANKI_RESOURCE_LOGE("Non-builtins should have a value: %s", &in.m_name[0]);
				return ErrorCode::USER_DATA;
			}

			if(in.m_flags.m_const)
			{
				ANKI_RESOURCE_LOGE("Consts should have a value: %s", &in.m_name[0]);
				return ErrorCode::USER_DATA;
			}
		}

		// <inShadow>
		ANKI_CHECK(inputEl.getChildElementOptional("inShadow", el));
		if(el)
		{
			I64 tmp;
			ANKI_CHECK(el.getI64(tmp));
			in.m_flags.m_inShadow = tmp;
		}
		else
		{
			in.m_flags.m_inShadow = true;
		}

		// Set some stuff
		if(in.m_type >= ShaderVariableDataType::SAMPLERS_FIRST && in.m_type <= ShaderVariableDataType::SAMPLERS_LAST)
		{
			in.m_flags.m_texture = true;
		}
		else if(!in.m_flags.m_const)
		{
			in.m_flags.m_inBlock = true;
			m_uniformBlockReferencedMask |= glshaderbit;
		}

		// Now you have the info to check if duplicate
		Input* duplicateInp = nullptr;
		Error err = m_inputs.iterateForward([&duplicateInp, &in](Input& inp) -> Error {
			if(in.m_name == inp.m_name)
			{
				duplicateInp = &in;
				return ErrorCode::NONE;
			}

			return ErrorCode::NONE;
		});
		(void)err;

		if(duplicateInp != nullptr)
		{
			// Duplicate. Make sure it's the same as the other shader
			Bool same = duplicateInp->duplicate(in);

			if(!same)
			{
				ANKI_RESOURCE_LOGE("Variable defined differently between shaders: %s", &in.m_name[0]);
				return ErrorCode::USER_DATA;
			}

			duplicateInp->m_shaderDefinedMask |= glshaderbit;

			if(duplicateInp->m_flags.m_inBlock)
			{
				m_uniformBlockReferencedMask |= glshaderbit;
			}
		}
		else
		{
			in.m_shaderDefinedMask = glshaderbit;

			m_inputs.emplaceBack(std::move(in));
		}

		// Advance
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	return ErrorCode::NONE;
}

void MaterialLoader::processInputs()
{
	// Sort the variables to decrease the change of creating unique shaders
	m_inputs.sort([](const Input& a, const Input& b) { return a.m_name < b.m_name; });

	for(auto& in : m_inputs)
	{
		if(!in.m_flags.m_const)
		{
			// Handle NON-consts

			if(in.m_flags.m_texture)
			{
				// Not in block

				if(!in.m_flags.m_specialBuiltin)
				{
					in.m_binding = m_texBinding++;

					in.m_line.sprintf("layout(ANKI_TEX_BINDING(0, %u)) uniform %s tex%u;",
						in.m_binding,
						&in.typeStr()[0],
						in.m_binding);
				}
			}
			else if(in.m_flags.m_inBlock)
			{
				// In block

				in.m_index = m_nextIndex++;

				in.m_line.sprintf("#if %s\n"
								  "%s var%u%s;\n"
								  "#endif",
					!in.m_flags.m_inShadow ? "PASS == COLOR" : "1",
					&in.typeStr()[0],
					in.m_index,
					in.m_flags.m_instanced ? " INSTANCED" : "");
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}
		else
		{
			// Handle consts

			in.m_index = m_nextIndex++;

			String initList;
			in.m_value.join(m_alloc, ", ", initList);

			in.m_line.sprintf("const %s var%u = %s(%s);", &in.typeStr()[0], in.m_index, &in.typeStr()[0], &initList[0]);

			initList.destroy(m_alloc);
		}
	}
}

Error MaterialLoader::parseOperationTag(
	const XmlElement& operationTag, ShaderType glshader, ShaderTypeBit glshaderbit, String& out)
{
	CString cstr;
	XmlElement el;

	static const char* OUT = "out";

	CString funcName;
	StringListAuto argsList(m_alloc);

	// <id>
	I64 id;
	ANKI_CHECK(operationTag.getChildElement("id", el));
	ANKI_CHECK(el.getI64(id));

	// <returnType>
	XmlElement retTypeEl;
	ANKI_CHECK(operationTag.getChildElement("returnType", retTypeEl));
	ANKI_CHECK(retTypeEl.getText(cstr));
	Bool retTypeVoid = cstr == "void";

	// <function>
	ANKI_CHECK(operationTag.getChildElement("function", el));
	ANKI_CHECK(el.getText(funcName));

	// <arguments>
	XmlElement argsEl;
	ANKI_CHECK(operationTag.getChildElementOptional("arguments", argsEl));

	if(argsEl)
	{
		// Get all arguments
		XmlElement argEl;
		ANKI_CHECK(argsEl.getChildElement("argument", argEl));
		do
		{
			CString arg;
			ANKI_CHECK(argEl.getText(arg));

			// Search for all the inputs and mark the appropriate
			Input* input = nullptr;
			for(Input& in : m_inputs)
			{
				// Check that the first part of the string is equal to the variable and the following char is '['
				if(in.m_name == arg)
				{
					input = &in;
					in.m_shaderReferencedMask |= glshaderbit;
					break;
				}
			}

			// The argument should be an input variable or an outXX or global
			if(input == nullptr)
			{
				if(arg.find(OUT) != 0)
				{
					ANKI_RESOURCE_LOGE("Incorrect argument: %s", &arg[0]);
					return ErrorCode::USER_DATA;
				}
			}

			// Add to a list and do something special if instanced
			if(input)
			{
				if(!input->m_flags.m_specialBuiltin)
				{
					argsList.pushBackSprintf("%s%d%s",
						input->m_flags.m_texture ? "tex" : "var",
						!input->m_flags.m_texture ? input->m_index : input->m_binding,
						input->m_flags.m_instanced ? " INSTANCE_ID" : "");
				}
				else
				{
					ANKI_CHECK(argEl.getText(cstr));
					argsList.pushBackSprintf(&cstr[0]);
				}

				m_instanceIdMask |= glshaderbit;
			}
			else
			{
				ANKI_CHECK(argEl.getText(cstr));
				argsList.pushBackSprintf(&cstr[0]);
			}

			// Advance
			ANKI_CHECK(argEl.getNextSiblingElement("argument", argEl));
		} while(argEl);
	}

	// Now write everything
	StringListAuto lines(m_alloc);
	lines.pushBackSprintf("#if defined(%s_DEFINED)", &funcName[0]);

	// Write the defines for the operationOuts
	for(const String& arg : argsList)
	{
		if(arg.find(OUT) == 0)
		{
			lines.pushBackSprintf(" && defined(%s_DEFINED)", &arg[0]);
		}
	}
	lines.pushBackSprintf("\n");

	if(!retTypeVoid)
	{
		ANKI_CHECK(retTypeEl.getText(cstr));
		lines.pushBackSprintf("#define out%u_DEFINED\n"
							  "%s out%u = ",
			id,
			&cstr[0],
			id);
	}

	// write the "func(args...)" of "blah = func(args...)"
	StringAuto argsStr(m_alloc);

	if(!argsList.isEmpty())
	{
		argsList.join(m_alloc, ", ", argsStr);
	}

	lines.pushBackSprintf("%s(%s);\n#endif", &funcName[0], (argsStr.isEmpty()) ? "" : &argsStr[0]);

	// Bake final string
	lines.join(m_alloc, "", out);

	return ErrorCode::NONE;
}

void MaterialLoader::mutate(const RenderingKey& key)
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

	// Compute the block info for each var
	m_blockSize = 0;
	for(Input& in : m_inputs)
	{
		// Invalidate the var's block info
		in.m_blockInfo = ShaderVariableBlockInfo();

		if(!in.m_flags.m_inBlock)
		{
			continue;
		}

		if(pass == Pass::SM && !in.m_flags.m_inShadow)
		{
			continue;
		}

		// std140 rules
		in.m_blockInfo.m_offset = m_blockSize;
		in.m_blockInfo.m_arraySize = in.m_flags.m_instanced ? instanceCount : 1;

		if(in.m_type == ShaderVariableDataType::FLOAT)
		{
			in.m_blockInfo.m_arrayStride = sizeof(Vec4);

			if(in.m_blockInfo.m_arraySize == 1)
			{
				// No need to align the in.m_offset
				m_blockSize += sizeof(F32);
			}
			else
			{
				alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
				m_blockSize += sizeof(Vec4) * in.m_blockInfo.m_arraySize;
			}
		}
		else if(in.m_type == ShaderVariableDataType::VEC2)
		{
			in.m_blockInfo.m_arrayStride = sizeof(Vec4);

			if(in.m_blockInfo.m_arraySize == 1)
			{
				alignRoundUp(sizeof(Vec2), in.m_blockInfo.m_offset);
				m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec2);
			}
			else
			{
				alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
				m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec4) * in.m_blockInfo.m_arraySize;
			}
		}
		else if(in.m_type == ShaderVariableDataType::VEC3)
		{
			alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
			in.m_blockInfo.m_arrayStride = sizeof(Vec4);

			if(in.m_blockInfo.m_arraySize == 1)
			{
				m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec3);
			}
			else
			{
				m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec4) * in.m_blockInfo.m_arraySize;
			}
		}
		else if(in.m_type == ShaderVariableDataType::VEC4)
		{
			in.m_blockInfo.m_arrayStride = sizeof(Vec4);
			alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
			m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec4) * in.m_blockInfo.m_arraySize;
		}
		else if(in.m_type == ShaderVariableDataType::MAT3)
		{
			alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
			in.m_blockInfo.m_arrayStride = sizeof(Vec4) * 3;
			m_blockSize = in.m_blockInfo.m_offset + sizeof(Vec4) * 3 * in.m_blockInfo.m_arraySize;
			in.m_blockInfo.m_matrixStride = sizeof(Vec4);
		}
		else if(in.m_type == ShaderVariableDataType::MAT4)
		{
			alignRoundUp(sizeof(Vec4), in.m_blockInfo.m_offset);
			in.m_blockInfo.m_arrayStride = sizeof(Mat4);
			m_blockSize = in.m_blockInfo.m_offset + sizeof(Mat4) * in.m_blockInfo.m_arraySize;
			in.m_blockInfo.m_matrixStride = sizeof(Vec4);
		}
		else
		{
			ANKI_ASSERT(0);
		}
	}

	// Update the defines
	StringAuto defines(m_alloc);
	defines.sprintf("#define INSTANCE_COUNT %u\n"
					"#define TESSELATION %u\n"
					"#define LOD %u\n"
					"#define PASS %s\n",
		instanceCount,
		tessellation,
		lod,
		(pass == Pass::SM) ? "DEPTH" : "COLOR");

	// Merge strings
	for(U i = 0; i < m_sourceBaked.getSize(); i++)
	{
		if(!m_source[i].isEmpty())
		{
			m_sourceBaked[i].destroy();

			StringAuto tmp(m_alloc);
			m_source[i].join("\n", tmp);

			m_sourceBaked[i].sprintf("%s%s", &defines[0], &tmp[0]);
		}
	}
}

} // end namespace anki
