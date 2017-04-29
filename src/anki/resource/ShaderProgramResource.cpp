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
		ANKI_RESOURCE_LOGE("Incorrect type %s", (str) ? &str[0] : "<empty string>");
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

	for(ShaderProgramResourceInputVariable& var : m_inputVars)
	{
		for(auto& m : var.m_mutators)
		{
			m.m_values.destroy(alloc);
		}

		var.m_mutators.destroy(alloc);
		var.m_name.destroy(alloc);
	}
	m_inputVars.destroy(alloc);

	for(ShaderProgramResourceMutator& m : m_mutators)
	{
		m.m_name.destroy(alloc);
		m.m_values.destroy(alloc);
	}
	m_mutators.destroy(alloc);

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

	// <mutators>
	XmlElement mutatorsEl;
	ANKI_CHECK(rootEl.getChildElementOptional("mutators", mutatorsEl));
	if(mutatorsEl)
	{
		XmlElement mutatorEl;
		ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));
		U32 count = 0;
		ANKI_CHECK(mutatorEl.getSiblingElementsCount(count));
		++count;

		m_mutators.create(getAllocator(), count);
		count = 0;

		do
		{
			// Get name
			CString name;
			ANKI_CHECK(mutatorEl.getAttributeText("name", name));
			if(name.isEmpty())
			{
				ANKI_RESOURCE_LOGE("Mutator name is empty");
				return ErrorCode::USER_DATA;
			}

			// Check if already there
			for(U i = 0; i < count; ++i)
			{
				if(m_mutators[i].m_name == name)
				{
					ANKI_RESOURCE_LOGE("Mutator aleady present %s", &name[0]);
					return ErrorCode::USER_DATA;
				}
			}

			// Get values
			DynamicArrayAuto<ShaderProgramResourceMutatorValue> values(getTempAllocator());
			ANKI_CHECK(mutatorEl.getAttributeNumbers("values", values));
			if(values.getSize() < 1)
			{
				ANKI_RESOURCE_LOGE("Mutator doesn't have any values %s", &name[0]);
				return ErrorCode::USER_DATA;
			}

			std::sort(values.getBegin(), values.getEnd());
			for(U i = 1; i < values.getSize(); ++i)
			{
				if(values[i - 1] == values[i])
				{
					ANKI_RESOURCE_LOGE("Mutator %s has duplicate values %d", &name[0], values[i]);
					return ErrorCode::USER_DATA;
				}
			}

			// Init mutator
			m_mutators[count].m_name.create(getAllocator(), name);
			m_mutators[count].m_values.create(getAllocator(), values.getSize());
			for(U i = 0; i < values.getSize(); ++i)
			{
				m_mutators[count].m_values[i] = values[i];
			}

			++count;
			ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
		} while(mutatorEl);
	}

	// <shaders>
	XmlElement shadersEl;
	ANKI_CHECK(rootEl.getChildElement("shaders", shadersEl));

	// <shader>
	// Count the input variables
	U inputVarCount = 0;
	XmlElement shaderEl;
	ANKI_CHECK(shadersEl.getChildElement("shader", shaderEl));
	do
	{
		XmlElement inputsEl;
		ANKI_CHECK(shaderEl.getChildElementOptional("inputs", inputsEl));

		if(inputsEl)
		{
			XmlElement inputEl;
			ANKI_CHECK(inputsEl.getChildElement("input", inputEl));

			U32 count;
			ANKI_CHECK(inputEl.getSiblingElementsCount(count));
			++count;

			inputVarCount += count;
		}

		ANKI_CHECK(shaderEl.getNextSiblingElement("shader", shaderEl));
	} while(shaderEl);

	if(inputVarCount)
	{
		m_inputVars.create(getAllocator(), inputVarCount);
	}

	// <shader> again
	inputVarCount = 0;
	StringListAuto constSrcList(getTempAllocator());
	ShaderTypeBit presentShaders = ShaderTypeBit::NONE;
	Array<CString, 5> shaderSources;
	ANKI_CHECK(shadersEl.getChildElement("shader", shaderEl));
	do
	{
		// type
		CString shaderTypeTxt;
		ANKI_CHECK(shaderEl.getAttributeText("type", shaderTypeTxt));
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
			ANKI_CHECK(parseInputs(inputsEl, inputVarCount, constSrcList));
		}

		// <source>
		ANKI_CHECK(shaderEl.getChildElement("source", el));
		ANKI_CHECK(el.getText(shaderSources[shaderIdx]));

		// Advance
		ANKI_CHECK(shaderEl.getNextSiblingElement("shader", shaderEl));
	} while(shaderEl);

	ANKI_ASSERT(inputVarCount == m_inputVars.getSize());

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

	// Create sources
	StringAuto backedConstSrc(getTempAllocator());
	if(!constSrcList.isEmpty())
	{
		constSrcList.join("", backedConstSrc);
	}

	for(U s = 0; s < 5; ++s)
	{
		if(shaderSources[s])
		{
			ShaderLoader loader(&getManager());
			ANKI_CHECK(loader.parseSourceString(shaderSources[s]));

			if(!constSrcList.isEmpty())
			{
				m_sources[s].create(getAllocator(), backedConstSrc);
			}

			m_sources[s].append(getAllocator(), loader.getShaderSource());
		}
	}

	return ErrorCode::NONE;
}

Error ShaderProgramResource::parseInputs(XmlElement& inputsEl, U& inputVarCount, StringListAuto& constsSrc)
{
	XmlElement inputEl;
	ANKI_CHECK(inputsEl.getChildElement("input", inputEl));
	do
	{
		ShaderProgramResourceInputVariable& var = m_inputVars[inputVarCount];
		var.m_idx = inputVarCount;

		// name
		CString name;
		ANKI_CHECK(inputEl.getAttributeText("name", name));
		if(name.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Input variable name is empty");
			return ErrorCode::USER_DATA;
		}
		var.m_name.create(getAllocator(), name);

		// type
		CString typeTxt;
		ANKI_CHECK(inputEl.getAttributeText("type", typeTxt));
		ANKI_CHECK(computeShaderVariableDataType(typeTxt, var.m_dataType));

		// const
		Bool present;
		U32 constant = 0;
		ANKI_CHECK(inputEl.getAttributeNumberOptional("const", constant, present));
		var.m_const = constant != 0;

		// <mutators>
		XmlElement mutatorsEl;
		ANKI_CHECK(inputEl.getChildElementOptional("mutators", mutatorsEl));
		if(mutatorsEl)
		{
			XmlElement mutatorEl;
			ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

			U32 mutatorCount;
			ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
			++mutatorCount;
			ANKI_ASSERT(mutatorCount > 0);
			var.m_mutators.create(getAllocator(), mutatorCount);
			mutatorCount = 0;

			do
			{
				// name
				CString mutatorName;
				ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
				if(mutatorName.isEmpty())
				{
					ANKI_RESOURCE_LOGE("Empty mutator name in input variable %s", &name[0]);
					return ErrorCode::USER_DATA;
				}

				// Find the mutator
				const ShaderProgramResourceMutator* foundMutator = tryFindMutator(mutatorName);
				if(!foundMutator)
				{
					ANKI_RESOURCE_LOGE("Input variable %s can't link to unknown mutator %s", &name[0], &mutatorName[0]);
					return ErrorCode::USER_DATA;
				}

				// Get values
				DynamicArrayAuto<ShaderProgramResourceMutatorValue> vals(getTempAllocator());
				ANKI_CHECK(mutatorEl.getAttributeNumbers("values", vals));
				if(vals.getSize() < 1)
				{
					ANKI_RESOURCE_LOGE(
						"Mutator %s doesn't have any values for input variable %s", &mutatorName[0], &name[0]);
					return ErrorCode::USER_DATA;
				}

				// Sanity check values
				std::sort(vals.getBegin(), vals.getEnd());

				for(U i = 0; i < vals.getSize(); ++i)
				{
					auto val = vals[i];
					if(i > 0 && vals[i - 1] == val)
					{
						ANKI_RESOURCE_LOGE(
							"Mutator %s for input var %s has duplicate values", &mutatorName[0], &name[0]);
						return ErrorCode::USER_DATA;
					}

					if(!foundMutator->valueExists(val))
					{
						ANKI_RESOURCE_LOGE(
							"Mutator %s for input variable %s has a value that is not part of the mutator",
							&mutatorName[0],
							&name[0]);
						return ErrorCode::USER_DATA;
					}
				}

				// Set var
				var.m_mutators[mutatorCount].m_mutator = foundMutator;
				var.m_mutators[mutatorCount].m_values.create(getAllocator(), vals.getSize());
				for(U i = 0; i < vals.getSize(); ++i)
				{
					var.m_mutators[mutatorCount].m_values[i] = vals[i];
				}
				++mutatorCount;

				// Advance
				ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
			} while(mutatorEl);

			ANKI_ASSERT(mutatorCount == var.m_mutators.getSize());
		} // if(mutatorsEl)

		// instanceCount
		CString instanceCountTxt;
		Bool found;
		ANKI_CHECK(inputEl.getAttributeTextOptional("instanceCount", instanceCountTxt, found));
		if(found)
		{
			if(instanceCountTxt.isEmpty())
			{
				ANKI_RESOURCE_LOGE("instanceCount tag is empty for input variable %s", &name[0]);
				return ErrorCode::USER_DATA;
			}

			var.m_instanced = true;
			const ShaderProgramResourceMutator* instancingMutator = tryFindMutator(instanceCountTxt);
			if(!instancingMutator)
			{
				ANKI_RESOURCE_LOGE("Failed to find mutator %s, used as instanceCount attribute for input variable %s",
					&instanceCountTxt[0],
					&name[0]);
				return ErrorCode::USER_DATA;
			}

			if(!m_instancingMutator)
			{
				m_instancingMutator = instancingMutator;
			}
			else if(instancingMutator != m_instancingMutator)
			{
				ANKI_RESOURCE_LOGE("All input variables should have the same instancing mutator");
				return ErrorCode::USER_DATA;
			}
		}

		// Sanity checks
		if(var.isTexture() && var.m_const)
		{
			ANKI_RESOURCE_LOGE("Can't have a texture to be const: %s", &var.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		if(var.m_const && var.isInstanced())
		{
			ANKI_RESOURCE_LOGE("Can't have input variables that are instanced and const: %s", &var.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		if(var.isTexture() && var.isInstanced())
		{
			ANKI_RESOURCE_LOGE("Can't have texture that is instanced: %s", &var.m_name[0]);
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
			ANKI_RESOURCE_LOGE("Matrix input variable %s cannot be constant", &var.m_name[0]);
			return ErrorCode::USER_DATA;
		}

		// Append to consts source
		if(var.m_const)
		{
			if(var.m_mutators.getSize())
			{
				constsSrc.pushBack("#if ");
				compInputVarDefineString(var, constsSrc);
				constsSrc.pushBack("\n");
			}

			constsSrc.pushBackSprintf("const %s %s = %s(%s_const);\n", &typeTxt[0], &name[0], &typeTxt[0], &name[0]);

			if(var.m_mutators.getSize())
			{
				constsSrc.pushBack("#endif\n");
			}
		}

		// Advance
		++inputVarCount;
		ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
	} while(inputEl);

	return ErrorCode::NONE;
}

void ShaderProgramResource::compInputVarDefineString(
	const ShaderProgramResourceInputVariable& var, StringListAuto& list)
{
	if(var.m_mutators.getSize() > 0)
	{
		for(const ShaderProgramResourceInputVariable::Mutator& mutator : var.m_mutators)
		{
			list.pushBack("(");

			for(ShaderProgramResourceMutatorValue val : mutator.m_values)
			{
				list.pushBackSprintf("%s == %d || ", &mutator.m_mutator->m_name[0], int(val));
			}

			list.pushBack("0) && ");
		}

		list.pushBack("1");
	}
}

U64 ShaderProgramResource::computeVariantHash(WeakArray<const ShaderProgramResourceMutation> mutations,
	WeakArray<const ShaderProgramResourceConstantValue> constants) const
{
	U hash = 1;

	if(mutations.getSize())
	{
		hash = computeHash(&mutations[0], sizeof(mutations[0]) * mutations.getSize());
	}

	if(constants.getSize())
	{
		hash = appendHash(&constants[0], sizeof(constants[0]) * constants.getSize(), hash);
	}

	return hash;
}

void ShaderProgramResource::getOrCreateVariant(WeakArray<const ShaderProgramResourceMutation> mutation,
	WeakArray<const ShaderProgramResourceConstantValue> constants,
	const ShaderProgramResourceVariant*& variant) const
{
	// Sanity checks
	ANKI_ASSERT(mutation.getSize() == m_mutators.getSize());
	ANKI_ASSERT(mutation.getSize() <= 128 && "Wrong assumption");
	BitSet<128> mutatorPresent = {false};
	for(const ShaderProgramResourceMutation& m : mutation)
	{
		(void)m;
		ANKI_ASSERT(m.m_mutator);
		ANKI_ASSERT(m.m_mutator->valueExists(m.m_value));

		ANKI_ASSERT(&m_mutators[0] <= m.m_mutator);
		const U idx = m.m_mutator - &m_mutators[0];
		ANKI_ASSERT(idx < m_mutators.getSize());
		ANKI_ASSERT(!mutatorPresent.get(idx) && "Appeared 2 times in 'mutation'");
		mutatorPresent.set(idx);

#if ANKI_EXTRA_CHECKS
		if(m_instancingMutator == m.m_mutator)
		{
			ANKI_ASSERT(m.m_value > 0 && "Instancing value can't be negative");
		}
#endif
	}

	// Compute hash
	U64 hash = computeVariantHash(mutation, constants);

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
		initVariant(mutation, constants, *v);

		m_variants.pushBack(hash, v);
		variant = v;
	}
}

void ShaderProgramResource::initVariant(WeakArray<const ShaderProgramResourceMutation> mutations,
	WeakArray<const ShaderProgramResourceConstantValue> constants,
	ShaderProgramResourceVariant& variant) const
{
	variant.m_activeInputVars.unsetAll();
	variant.m_blockInfos.create(getAllocator(), m_inputVars.getSize());
	variant.m_texUnits.create(getAllocator(), m_inputVars.getSize());
	if(m_inputVars.getSize() > 0)
	{
		memorySet<I16>(&variant.m_texUnits[0], -1, variant.m_texUnits.getSize());
	}

	// Get instance count, one mutation has it
	U instanceCount = 1;
	if(m_instancingMutator)
	{
		for(const ShaderProgramResourceMutation& m : mutations)
		{
			if(m.m_mutator == m_instancingMutator)
			{
				ANKI_ASSERT(m.m_value > 0);
				instanceCount = m.m_value;
				break;
			}
		}
	}

	// - Compute the block info for each var
	// - Activate vars
	// - Compute varius strings
	StringListAuto blockCode(getTempAllocator());
	StringListAuto constsCode(getTempAllocator());
	StringListAuto texturesCode(getTempAllocator());
	StringListAuto globalsCode(getTempAllocator());
	U texUnit = 0;

	for(const ShaderProgramResourceInputVariable& in : m_inputVars)
	{
		if(!in.acceptAllMutations(mutations))
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
			blockInfo.m_arraySize = (in.m_instanced) ? instanceCount : 1;

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

			if(in.m_instanced && instanceCount > 1)
			{
				blockCode.pushBackSprintf(
					"%s %s_i[%s];\n", &toString(in.m_dataType)[0], &in.m_name[0], &m_instancingMutator->getName()[0]);

				globalsCode.pushBackSprintf(R"(#if defined(ANKI_VERTEX_SHADER)
%s %s = %s_i[gl_InstanceID];
#else
// TODO
#endif
)",
					&toString(in.m_dataType)[0],
					&in.m_name[0],
					&in.m_name[0]);
			}
			else
			{
				blockCode.pushBackSprintf("%s %s;\n", &toString(in.m_dataType)[0], &in.m_name[0]);
			}
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

	for(const ShaderProgramResourceMutation& m : mutations)
	{
		shaderHeaderSrc.pushBackSprintf("#define %s %d\n", &m.m_mutator->getName()[0], m.m_value);
	}

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

		shaderHeaderSrc.pushBack("layout(ANKI_UBO_BINDING(0, 0), std140, row_major) uniform sprubo00_ {\n");
		shaderHeaderSrc.pushBack(str.toCString());
		shaderHeaderSrc.pushBack("};\n");
	}

	if(texturesCode)
	{
		StringAuto str(getTempAllocator());
		texturesCode.join("", str);
		shaderHeaderSrc.pushBack(str.toCString());
	}

	if(globalsCode)
	{
		StringAuto str(getTempAllocator());
		globalsCode.join("", str);
		shaderHeaderSrc.pushBack(str.toCString());
	}

	StringAuto shaderHeader(getTempAllocator());
	shaderHeaderSrc.join("", shaderHeader);

	// Create the shaders and the program
	Array<ShaderPtr, 5> shaders;
	for(ShaderType i = ShaderType::FIRST_GRAPHICS; i <= ShaderType::LAST_GRAPHICS; ++i)
	{
		if(!m_sources[i])
		{
			continue;
		}

		StringAuto src(getTempAllocator());
		src.append(shaderHeader);
		src.append(m_sources[i]);

		shaders[i] = getManager().getGrManager().newInstance<Shader>(i, src.toCString());
	}

	variant.m_prog = getManager().getGrManager().newInstance<ShaderProgram>(shaders[ShaderType::VERTEX],
		shaders[ShaderType::TESSELLATION_CONTROL],
		shaders[ShaderType::TESSELLATION_EVALUATION],
		shaders[ShaderType::GEOMETRY],
		shaders[ShaderType::FRAGMENT]);
}

} // end namespace anki
