// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResource.h>
#include <anki/resource/ResourceManager.h>
#include <anki/util/Filesystem.h>
#include <tinyexpr.h>

namespace anki
{

Bool ShaderProgramResourceInputVariable::evalPreproc(ConstWeakArray<te_variable> vars) const
{
	ANKI_ASSERT(vars.getSize() == m_program->getMutators().getSize());

	int err;
	te_expr* n = te_compile(m_preprocExpr.cstr(), &vars[0], vars.getSize(), &err);

	if(!n)
	{
		ANKI_RESOURCE_LOGF("Error evaluating expression: %s", m_preprocExpr.cstr());
	}

	F64 evalOut = te_eval(n);
	if(evalOut != 0.0 && evalOut != 1.0)
	{
		ANKI_RESOURCE_LOGF("Wrong result of the expression: %s", m_preprocExpr.cstr());
	}

	te_free(n);
	return evalOut != 0.0;
}

Bool ShaderProgramResourceInputVariable::recusiveSpin(U32 crntMissingMutatorIdx,
	WeakArray<const ShaderProgramResourceMutator*> missingMutators,
	WeakArray<te_variable> vars,
	WeakArray<F64> varValues,
	U32 varIdxOffset) const
{
	for(ShaderProgramResourceMutatorValue val : missingMutators[crntMissingMutatorIdx]->getValues())
	{
		const U valIdx = varIdxOffset + crntMissingMutatorIdx;
		varValues[valIdx] = val;
		vars[valIdx] = {missingMutators[crntMissingMutatorIdx]->getName().cstr(), &varValues[valIdx], 0, 0};

		if(crntMissingMutatorIdx == missingMutators.getSize() - 1)
		{
			// Last missing mutator will eval
			const Bool accepted = evalPreproc(vars);
			if(accepted)
			{
				return true;
			}
		}
		else
		{
			return recusiveSpin(crntMissingMutatorIdx + 1, missingMutators, vars, varValues, varIdxOffset);
		}
	}

	return false;
}

Bool ShaderProgramResourceInputVariable::acceptAllMutations(
	ConstWeakArray<ShaderProgramResourceMutation> mutations) const
{
	if(mutations.getSize() == 0 || !m_preprocExpr)
	{
		// Early exit
		return true;
	}

	static const U MAX_MUTATORS = 64;

	Array<const ShaderProgramResourceMutator*, MAX_MUTATORS> missingMutators;
	U missingMutatorCount = 0;

	Array<te_variable, MAX_MUTATORS> vars;
	Array<F64, MAX_MUTATORS> varValues;
	U varCount = 0;

	// - Find the mutators that don't appear in "mutations"
	// - Fill the vars with the known mutators
	for(const ShaderProgramResourceMutator& mutator : m_program->getMutators())
	{
		const ShaderProgramResourceMutation* foundMutation = nullptr;
		for(const ShaderProgramResourceMutation& mutation : mutations)
		{
			if(mutation.m_mutator == &mutator)
			{
				// Found the mutator
				foundMutation = &mutation;
				break;
			}
			else
			{
				ANKI_ASSERT(mutation.m_mutator->getName() != mutator.getName());
			}
		}

		if(foundMutation)
		{
			// Take the value from the mutation
			varValues[varCount] = foundMutation->m_value;
			vars[varCount] = {mutator.getName().cstr(), &varValues[varCount], 0, 0};
			++varCount;
		}
		else
		{
			missingMutators[missingMutatorCount++] = &mutator;
		}
	}
	ANKI_ASSERT(missingMutatorCount == m_program->getMutators().getSize() - varCount);

	// Eval the expression
	if(missingMutatorCount == 0)
	{
		ANKI_ASSERT(varCount == m_program->getMutators().getSize());
		return evalPreproc(ConstWeakArray<te_variable>(&vars[0], varCount));
	}
	else
	{
		// Spin
		return recusiveSpin(0,
			WeakArray<const ShaderProgramResourceMutator*>(&missingMutators[0], missingMutatorCount),
			WeakArray<te_variable>(&vars[0], m_program->getMutators().getSize()),
			WeakArray<F64>(&varValues[0], m_program->getMutators().getSize()),
			varCount);
	}

	ANKI_ASSERT(0 && "Shouldn't reach that");
	return false;
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
		auto it = m_variants.getBegin();
		ShaderProgramResourceVariant* variant = *it;
		m_variants.erase(getAllocator(), it);

		variant->m_blockInfos.destroy(alloc);
		variant->m_texUnits.destroy(alloc);
		alloc.deleteInstance(variant);
	}

	for(Input& var : m_inputVars)
	{
		var.m_name.destroy(alloc);
		var.m_preprocExpr.destroy(alloc);
	}
	m_inputVars.destroy(alloc);

	for(ShaderProgramResourceMutator& m : m_mutators)
	{
		m.m_name.destroy(alloc);
		m.m_values.destroy(alloc);
	}
	m_mutators.destroy(alloc);

	m_source.destroy(alloc);
}

Error ShaderProgramResource::load(const ResourceFilename& filename, Bool async)
{
	// Preprocess
	ShaderProgramPreprocessor pp(filename, &getManager().getFilesystem(), getTempAllocator());
	ANKI_CHECK(pp.parse());

	// Create the source
	m_source.create(getAllocator(), pp.getSource());

	// Create the mutators
	U instancedMutatorIdx = MAX_U;
	if(pp.getMutators().getSize())
	{
		m_mutators.create(getAllocator(), pp.getMutators().getSize());

		for(U i = 0; i < m_mutators.getSize(); ++i)
		{
			Mutator& out = m_mutators[i];
			const ShaderProgramPreprocessorMutator& in = pp.getMutators()[i];

			out.m_name.create(getAllocator(), in.getName());
			out.m_values.create(getAllocator(), in.getValues().getSize());
			for(U j = 0; j < out.m_values.getSize(); ++j)
			{
				out.m_values[j] = in.getValues()[j];
			}

			if(in.isInstanced())
			{
				instancedMutatorIdx = i;
			}
		}
	}

	// Create the inputs
	if(pp.getInputs().getSize())
	{
		m_inputVars.create(getAllocator(), pp.getInputs().getSize());

		for(U i = 0; i < m_inputVars.getSize(); ++i)
		{
			Input& out = m_inputVars[i];
			const ShaderProgramPreprocessorInput& in = pp.getInputs()[i];

			out.m_program = this;
			out.m_name.create(getAllocator(), in.getName());
			if(in.getPreprocessorCondition())
			{
				out.m_preprocExpr.create(getAllocator(), in.getPreprocessorCondition());
			}
			out.m_idx = i;
			out.m_dataType = in.getDataType();
			out.m_const = in.isConstant();
			out.m_instanced = in.isInstanced();
		}
	}

	// Set some other vars
	m_descriptorSet = pp.getDescritproSet();
	m_shaderStages = pp.getShaderStages();
	if(instancedMutatorIdx != MAX_U)
	{
		m_instancingMutator = &m_mutators[instancedMutatorIdx];
	}

	return Error::NONE;
}

U64 ShaderProgramResource::computeVariantHash(ConstWeakArray<ShaderProgramResourceMutation> mutations,
	ConstWeakArray<ShaderProgramResourceConstantValue> constants) const
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

void ShaderProgramResource::getOrCreateVariant(ConstWeakArray<ShaderProgramResourceMutation> mutation,
	ConstWeakArray<ShaderProgramResourceConstantValue> constants,
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
		variant = *it;
	}
	else
	{
		// Create one
		ShaderProgramResourceVariant* v = getAllocator().newInstance<ShaderProgramResourceVariant>();
		initVariant(mutation, constants, *v);

		m_variants.emplace(getAllocator(), hash, v);
		variant = v;
	}
}

void ShaderProgramResource::initVariant(ConstWeakArray<ShaderProgramResourceMutation> mutations,
	ConstWeakArray<ShaderProgramResourceConstantValue> constants,
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
	StringListAuto headerSrc(getTempAllocator());
	U texUnit = 0;

	for(const Input& in : m_inputVars)
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

			if(in.m_dataType == ShaderVariableDataType::FLOAT || in.m_dataType == ShaderVariableDataType::INT
				|| in.m_dataType == ShaderVariableDataType::UINT)
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
			else if(in.m_dataType == ShaderVariableDataType::VEC2 || in.m_dataType == ShaderVariableDataType::IVEC2
					|| in.m_dataType == ShaderVariableDataType::UVEC2)
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
			else if(in.m_dataType == ShaderVariableDataType::VEC3 || in.m_dataType == ShaderVariableDataType::IVEC3
					|| in.m_dataType == ShaderVariableDataType::UVEC3)
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
			else if(in.m_dataType == ShaderVariableDataType::VEC4 || in.m_dataType == ShaderVariableDataType::IVEC4
					|| in.m_dataType == ShaderVariableDataType::UVEC4)
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
			case ShaderVariableDataType::INT:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %d\n", &in.m_name[0], constVal->m_int);
				break;
			case ShaderVariableDataType::IVEC2:
				headerSrc.pushBackSprintf(
					"#define %s_CONSTVAL %d, %d\n", &in.m_name[0], constVal->m_ivec2.x(), constVal->m_ivec2.y());
				break;
			case ShaderVariableDataType::IVEC3:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %d, %d, %d\n",
					&in.m_name[0],
					constVal->m_ivec3.x(),
					constVal->m_ivec3.y(),
					constVal->m_ivec3.z());
				break;
			case ShaderVariableDataType::IVEC4:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %d, %d, %d, %d\n",
					&in.m_name[0],
					constVal->m_ivec4.x(),
					constVal->m_ivec4.y(),
					constVal->m_ivec4.z(),
					constVal->m_ivec4.w());
				break;
			case ShaderVariableDataType::UINT:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %u\n", &in.m_name[0], constVal->m_uint);
				break;
			case ShaderVariableDataType::UVEC2:
				headerSrc.pushBackSprintf(
					"#define %s_CONSTVAL %u, %u\n", &in.m_name[0], constVal->m_uvec2.x(), constVal->m_uvec2.y());
				break;
			case ShaderVariableDataType::UVEC3:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %u, %u, %u\n",
					&in.m_name[0],
					constVal->m_uvec3.x(),
					constVal->m_uvec3.y(),
					constVal->m_uvec3.z());
				break;
			case ShaderVariableDataType::UVEC4:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %u, %u, %u, %u\n",
					&in.m_name[0],
					constVal->m_uvec4.x(),
					constVal->m_uvec4.y(),
					constVal->m_uvec4.z(),
					constVal->m_uvec4.w());
				break;
			case ShaderVariableDataType::FLOAT:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %f\n", &in.m_name[0], constVal->m_float);
				break;
			case ShaderVariableDataType::VEC2:
				headerSrc.pushBackSprintf(
					"#define %s_CONSTVAL %f, %f\n", &in.m_name[0], constVal->m_vec2.x(), constVal->m_vec2.y());
				break;
			case ShaderVariableDataType::VEC3:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %f, %f, %f\n",
					&in.m_name[0],
					constVal->m_vec3.x(),
					constVal->m_vec3.y(),
					constVal->m_vec3.z());
				break;
			case ShaderVariableDataType::VEC4:
				headerSrc.pushBackSprintf("#define %s_CONSTVAL %f, %f, %f, %f\n",
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
			headerSrc.pushBackSprintf("#define %s_TEXUNIT %u\n", &in.m_name[0], texUnit);
			variant.m_texUnits[in.m_idx] = texUnit;
			++texUnit;
		}
	}

	// Check if we can use push constants
	variant.m_usesPushConstants =
		instanceCount == 1
		&& variant.m_uniBlockSize <= getManager().getGrManager().getDeviceCapabilities().m_pushConstantsSize;

	// Write the source header
	StringListAuto shaderHeaderSrc(getTempAllocator());
	shaderHeaderSrc.pushBackSprintf("#define USE_PUSH_CONSTANTS %d\n", I(variant.m_usesPushConstants));

	for(const ShaderProgramResourceMutation& m : mutations)
	{
		shaderHeaderSrc.pushBackSprintf("#define %s %d\n", &m.m_mutator->getName()[0], m.m_value);
	}

	if(!headerSrc.isEmpty())
	{
		StringAuto str(getTempAllocator());
		headerSrc.join("", str);
		shaderHeaderSrc.pushBack(str.toCString());
	}

	StringAuto shaderHeader(getTempAllocator());
	shaderHeaderSrc.join("", shaderHeader);

	// Create the program name
	StringAuto progName(getTempAllocator());
	getFilepathFilename(getFilename(), progName);
	char* cprogName = const_cast<char*>(progName.cstr());
	if(progName.getLength() > MAX_GR_OBJECT_NAME_LENGTH)
	{
		cprogName[MAX_GR_OBJECT_NAME_LENGTH] = '\0';
	}

	// Create the shaders and the program
	ShaderProgramInitInfo progInf(cprogName);
	for(ShaderType i = ShaderType::FIRST; i < ShaderType::COUNT; ++i)
	{
		if(!(m_shaderStages & ShaderTypeBit(1 << U(i))))
		{
			continue;
		}

		StringAuto src(getTempAllocator());
		src.append(shaderHeader);
		src.append(m_source);

		// Compile
		DynamicArrayAuto<U8> bin(getTempAllocator());

		ShaderCompilerOptions compileOptions;
		compileOptions.setFromGrManager(getManager().getGrManager());
		compileOptions.m_shaderType = i;

		Error err = getManager().getShaderCompiler().compile(src.toCString(), nullptr, compileOptions, bin);
		if(err)
		{
			ANKI_RESOURCE_LOGF("Shader compilation failed");
		}

		ShaderInitInfo inf(cprogName);
		inf.m_shaderType = i;
		inf.m_binary = ConstWeakArray<U8>(&bin[0], bin.getSize());

		progInf.m_shaders[i] = getManager().getGrManager().newShader(inf);
	}

	variant.m_prog = getManager().getGrManager().newShaderProgram(progInf);
}

} // end namespace anki
