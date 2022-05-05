// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/Xml.h>

namespace anki {

static const Array<CString, U32(BuiltinMutatorId::COUNT)> BUILTIN_MUTATOR_NAMES = {
	{"NONE", "ANKI_TECHNIQUE", "ANKI_LOD", "ANKI_BONES", "ANKI_VELOCITY"}};

static const Array<CString, U(RenderingTechnique::COUNT)> TECHNIQUE_NAMES = {
	{"GBuffer", "GBufferEarlyZ", "Shadow", "Forward", "RtShadow"}};

// This is some trickery to select calling between XmlElement::getAttributeNumber and XmlElement::getAttributeNumbers
namespace {

template<typename T>
class IsShaderVarDataTypeAnArray
{
public:
	static constexpr Bool VALUE = false;
};

#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount, isIntagralType) \
	template<> \
	class IsShaderVarDataTypeAnArray<type> \
	{ \
	public: \
		static constexpr Bool VALUE = rowCount * columnCount > 1; \
	};
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO

template<typename T, Bool isArray = IsShaderVarDataTypeAnArray<T>::VALUE>
class GetAttribute
{
public:
	Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumbers("value", out);
	}
};

template<typename T>
class GetAttribute<T, false>
{
public:
	Error operator()(const XmlElement& el, T& out)
	{
		return el.getAttributeNumber("value", out);
	}
};

} // namespace

MaterialVariable::MaterialVariable()
{
	m_Mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

MaterialResource::MaterialResource(ResourceManager* manager)
	: ResourceObject(manager)
{
	memset(m_techniqueToProgram.getBegin(), 0xFF, m_techniqueToProgram.getSizeInBytes());
}

MaterialResource::~MaterialResource()
{
	for(Program& p : m_programs)
	{
		p.m_partialMutation.destroy(getAllocator());
	}

	m_textures.destroy(getAllocator());

	for(MaterialVariable& var : m_vars)
	{
		var.m_name.destroy(getAllocator());
	}

	m_vars.destroy(getAllocator());
	m_programs.destroy(getAllocator());

	getAllocator().deallocate(m_prefilledLocalUniforms, 0);
}

const MaterialVariable* MaterialResource::tryFindVariableInternal(CString name) const
{
	for(const MaterialVariable& v : m_vars)
	{
		if(v.m_name == name)
		{
			return &v;
		}
	}

	return nullptr;
}

Error MaterialResource::load(const ResourceFilename& filename, Bool async)
{
	XmlDocument doc;
	XmlElement el;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("material", rootEl));

	// <shaderPrograms>
	XmlElement shaderProgramsEl;
	ANKI_CHECK(rootEl.getChildElement("shaderPrograms", shaderProgramsEl));
	XmlElement shaderProgramEl;
	ANKI_CHECK(shaderProgramsEl.getChildElement("shaderProgram", shaderProgramEl));
	do
	{
		ANKI_CHECK(parseShaderProgram(shaderProgramEl, async));
		ANKI_CHECK(shaderProgramEl.getNextSiblingElement("shaderProgram", shaderProgramEl));
	} while(shaderProgramEl);

	ANKI_ASSERT(!!m_techniquesMask);

	// <inputs>
	BitSet<128> varsSet(false);
	ANKI_CHECK(rootEl.getChildElementOptional("inputs", el));
	if(el)
	{
		XmlElement inputEl;
		ANKI_CHECK(el.getChildElement("input", inputEl));
		do
		{
			ANKI_CHECK(parseInput(inputEl, async, varsSet));
			ANKI_CHECK(inputEl.getNextSiblingElement("input", inputEl));
		} while(inputEl);
	}

	if(varsSet.getEnabledBitCount() != m_vars.getSize())
	{
		ANKI_RESOURCE_LOGE("Forgot to set a default value in %u input variables",
						   U32(m_vars.getSize() - varsSet.getEnabledBitCount()));
		return Error::USER_DATA;
	}

	prefillLocalUniforms();

	return Error::NONE;
}

Error MaterialResource::parseShaderProgram(XmlElement shaderProgramEl, Bool async)
{
	// filename
	CString fname;
	ANKI_CHECK(shaderProgramEl.getAttributeText("filename", fname));

	if(fname.find("/Rt") != CString::NPOS && !getManager().getGrManager().getDeviceCapabilities().m_rayTracingEnabled)
	{
		// Skip RT programs when RT is disabled
		return Error::NONE;
	}

	Program& prog = *m_programs.emplaceBack(getAllocator());
	ANKI_CHECK(getManager().loadResource(fname, prog.m_prog, async));

	// <mutation>
	XmlElement mutatorsEl;
	ANKI_CHECK(shaderProgramEl.getChildElementOptional("mutation", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl, prog));
	}

	// And find the builtin mutators
	ANKI_CHECK(findBuiltinMutators(prog));

	// Create the vars
	ANKI_CHECK(createVars(prog));

	return Error::NONE;
}

Error MaterialResource::createVars(Program& prog)
{
	const ShaderProgramBinary& binary = prog.m_prog->getBinary();

	// Find struct
	const ShaderProgramBinaryStruct* localUniformsStruct = nullptr;
	for(const ShaderProgramBinaryStruct& strct : binary.m_structs)
	{
		if(CString(strct.m_name.getBegin()) == "AnKiLocalUniforms")
		{
			localUniformsStruct = &strct;
			break;
		}

		++prog.m_localUniformsStructIdx;
	}

	if(localUniformsStruct == nullptr)
	{
		prog.m_localUniformsStructIdx = MAX_U32;
	}

	// Iterate all members of the local uniforms struct to add its members
	U32 offsetof = 0;
	for(U32 i = 0; localUniformsStruct && i < localUniformsStruct->m_members.getSize(); ++i)
	{
		const ShaderProgramBinaryStructMember& member = localUniformsStruct->m_members[i];
		const CString memberName = member.m_name.getBegin();

		// Check if it needs to be added
		Bool addIt = false;
		if(member.m_dependentMutator == MAX_U32)
		{
			addIt = true;
		}
		else
		{
			Bool found = false;
			for(const PartialMutation& m : prog.m_partialMutation)
			{
				if(m.m_mutator->m_name == binary.m_mutators[member.m_dependentMutator].m_name.getBegin())
				{
					if(m.m_value == member.m_dependentMutatorValue)
					{
						addIt = true;
					}
					found = true;
					break;
				}
			}

			if(!found)
			{
				ANKI_RESOURCE_LOGE("Incorrect combination of member variable %s and dependent mutator %s",
								   memberName.cstr(), binary.m_mutators[member.m_dependentMutator].m_name.getBegin());
				return Error::USER_DATA;
			}
		}

		if(addIt)
		{
			MaterialVariable* var = tryFindVariable(memberName);
			if(var)
			{
				if(var->m_dataType != member.m_type || var->m_offsetInLocalUniforms != offsetof)
				{
					ANKI_RESOURCE_LOGE("Member variable doesn't match between techniques: %s", memberName.cstr());
					return Error::USER_DATA;
				}
			}
			else
			{
				// Check that there are no other vars that overlap with the current var. This could happen if
				// different programs have different signature for AnKiLocalUniforms
				for(const MaterialVariable& otherVar : m_vars)
				{
					if(!otherVar.isUniform())
					{
						continue;
					}

					const U32 aVarOffset = otherVar.m_offsetInLocalUniforms;
					const U32 aVarEnd = aVarOffset + getShaderVariableDataTypeInfo(otherVar.m_dataType).m_size;
					const U32 bVarOffset = offsetof;
					const U32 bVarEnd = bVarOffset + getShaderVariableDataTypeInfo(member.m_type).m_size;

					if((aVarOffset <= bVarOffset && aVarEnd > bVarOffset)
					   || (bVarOffset <= aVarOffset && bVarEnd > aVarOffset))
					{
						ANKI_RESOURCE_LOGE("Member %s in AnKiLocalUniforms overlaps with %s. Check your shaders",
										   memberName.cstr(), otherVar.m_name.cstr());
						return Error::USER_DATA;
					}
				}

				// All good, add it
				var = m_vars.emplaceBack(getAllocator());
				var->m_name.create(getAllocator(), memberName);
				var->m_offsetInLocalUniforms = offsetof;
				var->m_dataType = member.m_type;

				offsetof += getShaderVariableDataTypeInfo(member.m_type).m_size;
			}
		}
	}

	m_localUniformsSize = max(offsetof, m_localUniformsSize);

	// Iterate all variants of builtin mutators to gather the opaques
	ShaderProgramResourceVariantInitInfo initInfo(prog.m_prog);

	for(const PartialMutation& m : prog.m_partialMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name, m.m_value);
	}

	Array<const ShaderProgramResourceMutator*, U(BuiltinMutatorId::COUNT)> mutatorPtrs = {};
	for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
	{
		mutatorPtrs[id] = prog.m_prog->tryFindMutator(BUILTIN_MUTATOR_NAMES[id]);
	}

#define ANKI_LOOP(builtIn) \
	for(U32 i = 0; \
		i \
		< ((mutatorPtrs[BuiltinMutatorId::builtIn]) ? mutatorPtrs[BuiltinMutatorId::builtIn]->m_values.getSize() : 1); \
		++i) \
	{ \
		if(mutatorPtrs[BuiltinMutatorId::builtIn]) \
		{ \
			initInfo.addMutation(mutatorPtrs[BuiltinMutatorId::builtIn]->m_name, \
								 mutatorPtrs[BuiltinMutatorId::builtIn]->m_values[i]); \
		}

#define ANKI_LOOP_END() }

	ANKI_LOOP(TECHNIQUE)
	ANKI_LOOP(LOD)
	ANKI_LOOP(BONES)
	ANKI_LOOP(VELOCITY)
	{
		const ShaderProgramResourceVariant* variant;
		prog.m_prog->getOrCreateVariant(initInfo, variant);

		// Add opaque vars
		for(const ShaderProgramBinaryOpaqueInstance& instance : variant->getBinaryVariant().m_opaques)
		{
			const ShaderProgramBinaryOpaque& opaque = binary.m_opaques[instance.m_index];
			if(opaque.m_type == ShaderVariableDataType::SAMPLER)
			{
				continue;
			}

			const CString opaqueName = opaque.m_name.getBegin();
			MaterialVariable* var = tryFindVariable(opaqueName);

			if(var)
			{
				if(var->m_dataType != opaque.m_type || var->m_opaqueBinding != opaque.m_binding)
				{
					ANKI_RESOURCE_LOGE("Opaque variable doesn't match between techniques: %s", opaqueName.cstr());
					return Error::USER_DATA;
				}
			}
			else
			{
				// Check that there are no other opaque with the same binding
				for(const MaterialVariable& otherVar : m_vars)
				{
					if(!otherVar.isBoundableTexture())
					{
						continue;
					}

					if(otherVar.m_opaqueBinding == opaque.m_binding)
					{
						ANKI_RESOURCE_LOGE("Opaque variable %s has the same binding as %s. Check your shaders",
										   otherVar.m_name.cstr(), opaqueName.cstr());
						return Error::USER_DATA;
					}
				}

				// All good, add it
				var = m_vars.emplaceBack(getAllocator());
				var->m_name.create(getAllocator(), opaqueName);
				var->m_opaqueBinding = opaque.m_binding;
				var->m_dataType = opaque.m_type;
			}
		}
	}
	ANKI_LOOP_END()
	ANKI_LOOP_END()
	ANKI_LOOP_END()
	ANKI_LOOP_END()

#undef ANKI_LOOP
#undef ANKI_LOOP_END

	return Error::NONE;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl, Program& prog)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	prog.m_partialMutation.create(getAllocator(), mutatorCount);
	mutatorCount = 0;

	do
	{
		PartialMutation& pmutation = prog.m_partialMutation[mutatorCount];

		// name
		CString mutatorName;
		ANKI_CHECK(mutatorEl.getAttributeText("name", mutatorName));
		if(mutatorName.isEmpty())
		{
			ANKI_RESOURCE_LOGE("Mutator name is empty");
			return Error::USER_DATA;
		}

		for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
		{
			if(id == BuiltinMutatorId::NONE)
			{
				continue;
			}

			if(mutatorName == BUILTIN_MUTATOR_NAMES[id])
			{
				ANKI_RESOURCE_LOGE("Materials shouldn't list builtin mutators: %s", mutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		if(mutatorName.find("ANKI_") == 0)
		{
			ANKI_RESOURCE_LOGE("Mutators can't start with ANKI_: %s", mutatorName.cstr());
			return Error::USER_DATA;
		}

		// value
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", pmutation.m_value));

		// Find mutator
		pmutation.m_mutator = prog.m_prog->tryFindMutator(mutatorName);

		if(!pmutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::USER_DATA;
		}

		if(!pmutation.m_mutator->valueExists(pmutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", pmutation.m_value, mutatorName.cstr());
			return Error::USER_DATA;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	ANKI_ASSERT(mutatorCount == prog.m_partialMutation.getSize());

	return Error::NONE;
}

Error MaterialResource::findBuiltinMutators(Program& prog)
{
	U builtinMutatorCount = 0;

	// ANKI_TECHNIQUE
	CString techniqueMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::TECHNIQUE];
	const ShaderProgramResourceMutator* techniqueMutator = prog.m_prog->tryFindMutator(techniqueMutatorName);
	if(techniqueMutator)
	{
		for(U32 i = 0; i < techniqueMutator->m_values.getSize(); ++i)
		{
			const MutatorValue mvalue = techniqueMutator->m_values[i];
			if(mvalue >= MutatorValue(RenderingTechnique::COUNT) || mvalue < MutatorValue(RenderingTechnique::FIRST))
			{
				ANKI_RESOURCE_LOGE("Mutator %s has a wrong value %d", techniqueMutatorName.cstr(), mvalue);
				return Error::USER_DATA;
			}

			const RenderingTechnique techniqueId = RenderingTechnique(mvalue);
			const PtrSize progIdx = &prog - m_programs.getBegin();
			ANKI_ASSERT(progIdx < m_programs.getSize());
			m_techniqueToProgram[techniqueId] = U8(progIdx);

			const RenderingTechniqueBit mask = RenderingTechniqueBit(1 << techniqueId);
			if(!!(m_techniquesMask & mask))
			{
				ANKI_RESOURCE_LOGE("The %s technique appeared more than once", TECHNIQUE_NAMES[mvalue].cstr());
				return Error::USER_DATA;
			}
			m_techniquesMask |= mask;
		}

		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::TECHNIQUE);
	}
	else
	{
		ANKI_RESOURCE_LOGE("Mutator %s should be present in every shader program referenced by a material",
						   techniqueMutatorName.cstr());
		return Error::USER_DATA;
	}

	// ANKI_LOD
	CString lodMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD];
	const ShaderProgramResourceMutator* lodMutator = prog.m_prog->tryFindMutator(lodMutatorName);
	if(lodMutator)
	{
		if(lodMutator->m_values.getSize() > MAX_LOD_COUNT)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have at least %u values in the program", lodMutatorName.cstr(),
							   U32(MAX_LOD_COUNT));
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < lodMutator->m_values.getSize(); ++i)
		{
			if(lodMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   lodMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		prog.m_lodCount = U8(lodMutator->m_values.getSize());
		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::LOD);
	}

	// ANKI_BONES
	CString bonesMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES];
	const ShaderProgramResourceMutator* bonesMutator = prog.m_prog->tryFindMutator(bonesMutatorName);
	if(bonesMutator)
	{
		if(bonesMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", bonesMutatorName.cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < bonesMutator->m_values.getSize(); ++i)
		{
			if(bonesMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   bonesMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;

		// Find if the bindings are present
		ConstWeakArray<ShaderProgramBinaryBlock> storageBlocks = prog.m_prog->getBinary().m_storageBlocks;
		U foundCount = 0;
		for(U32 i = 0; i < storageBlocks.getSize(); ++i)
		{
			const U32 binding = storageBlocks[i].m_binding;
			const U32 set = storageBlocks[i].m_set;
			if((binding == MATERIAL_BINDING_BONE_TRANSFORMS || binding == MATERIAL_BINDING_PREVIOUS_BONE_TRANSFORMS)
			   && set == MATERIAL_SET_LOCAL)
			{
				++foundCount;
			}
		}

		if(foundCount != 2)
		{
			ANKI_RESOURCE_LOGE("The shader has %s mutation but not the storage buffers for the bone transforms",
							   bonesMutatorName.cstr());
			return Error::USER_DATA;
		}

		m_supportsSkinning = true;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::BONES);
	}

	// VELOCITY
	CString velocityMutatorName = BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY];
	const ShaderProgramResourceMutator* velocityMutator = prog.m_prog->tryFindMutator(velocityMutatorName);
	if(velocityMutator)
	{
		if(velocityMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", velocityMutatorName.cstr());
			return Error::USER_DATA;
		}

		for(U32 i = 0; i < velocityMutator->m_values.getSize(); ++i)
		{
			if(velocityMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   velocityMutatorName.cstr());
				return Error::USER_DATA;
			}
		}

		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::VELOCITY);
	}

	if(prog.m_partialMutation.getSize() + builtinMutatorCount != prog.m_prog->getMutators().getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::USER_DATA;
	}

	return Error::NONE;
}

Error MaterialResource::parseInput(XmlElement inputEl, Bool async, BitSet<128>& varsSet)
{
	// Get var name
	CString varName;
	ANKI_CHECK(inputEl.getAttributeText("name", varName));

	// Try find var
	MaterialVariable* foundVar = tryFindVariable(varName);
	if(!foundVar)
	{
		ANKI_RESOURCE_LOGE("Input name is wrong, variable not found: %s", varName.cstr());
		return Error::USER_DATA;
	}

	// Set check
	const U32 idx = U32(foundVar - m_vars.getBegin());
	if(varsSet.get(idx))
	{
		ANKI_RESOURCE_LOGE("Input already has a value: %s", varName.cstr());
		return Error::USER_DATA;
	}
	varsSet.set(idx);

	// Set the value
	if(foundVar->isBoundableTexture())
	{
		CString texfname;
		ANKI_CHECK(inputEl.getAttributeText("value", texfname));
		ANKI_CHECK(getManager().loadResource(texfname, foundVar->m_image, async));

		m_textures.emplaceBack(getAllocator(), foundVar->m_image->getTexture());
	}
	else if(foundVar->m_dataType == ShaderVariableDataType::U32)
	{
		// U32 is a bit special. It might be a number or a bindless texture

		CString value;
		ANKI_CHECK(inputEl.getAttributeText("value", value));

		// Check if the value has letters
		Bool containsAlpharithmetic = false;
		for(Char c : value)
		{
			if((c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'a'))
			{
				containsAlpharithmetic = true;
				break;
			}
		}

		// If it has letters it's a texture
		if(containsAlpharithmetic)
		{
			ANKI_CHECK(getManager().loadResource(value, foundVar->m_image, async));

			foundVar->m_U32 = foundVar->m_image->getTextureView()->getOrCreateBindlessTextureIndex();
		}
		else
		{
			ANKI_CHECK(GetAttribute<U32>()(inputEl, foundVar->m_U32));
		}
	}
	else
	{
		switch(foundVar->m_dataType)
		{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::capital: \
		ANKI_CHECK(GetAttribute<type>()(inputEl, foundVar->ANKI_CONCATENATE(m_, type))); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	return Error::NONE;
}

void MaterialResource::prefillLocalUniforms()
{
	if(m_localUniformsSize == 0)
	{
		return;
	}

	m_prefilledLocalUniforms = getAllocator().allocate(m_localUniformsSize, 1);
	memset(m_prefilledLocalUniforms, 0, m_localUniformsSize);

	for(const MaterialVariable& var : m_vars)
	{
		if(!var.isUniform())
		{
			continue;
		}

		switch(var.m_dataType)
		{
#define ANKI_SVDT_MACRO(capital, type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::capital: \
		ANKI_ASSERT(var.m_offsetInLocalUniforms + sizeof(type) <= m_localUniformsSize); \
		memcpy(static_cast<U8*>(m_prefilledLocalUniforms) + var.m_offsetInLocalUniforms, &var.m_##type, sizeof(type)); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
		default:
			ANKI_ASSERT(0);
			break;
		}
	}
}

const MaterialVariant& MaterialResource::getOrCreateVariant(const RenderingKey& key_) const
{
	RenderingKey key = key_;
	ANKI_ASSERT(m_techniqueToProgram[key.getRenderingTechnique()] != MAX_U8);
	const Program& prog = m_programs[m_techniqueToProgram[key.getRenderingTechnique()]];

	key.setLod(min<U32>(prog.m_lodCount - 1, key.getLod()));

	ANKI_ASSERT(!key.getSkinned() || !!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::BONES)));
	ANKI_ASSERT(!key.getVelocity() || !!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::VELOCITY)));

	MaterialVariant& variant =
		prog.m_variantMatrix[key.getRenderingTechnique()][key.getLod()][key.getSkinned()][key.getVelocity()];

	// Check if it's initialized
	{
		RLockGuard<RWMutex> lock(prog.m_variantMatrixMtx);
		if(ANKI_LIKELY(variant.m_prog.isCreated()))
		{
			return variant;
		}
	}

	// Not initialized, init it
	WLockGuard<RWMutex> lock(prog.m_variantMatrixMtx);

	// Check again
	if(variant.m_prog.isCreated())
	{
		return variant;
	}

	ShaderProgramResourceVariantInitInfo initInfo(prog.m_prog);

	for(const PartialMutation& m : prog.m_partialMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name, m.m_value);
	}

	initInfo.addMutation(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::TECHNIQUE], MutatorValue(key.getRenderingTechnique()));

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::LOD)))
	{
		initInfo.addMutation(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::LOD], MutatorValue(key.getLod()));
	}

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::BONES)))
	{
		initInfo.addMutation(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::BONES], MutatorValue(key.getSkinned()));
	}

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::VELOCITY)))
	{
		initInfo.addMutation(BUILTIN_MUTATOR_NAMES[BuiltinMutatorId::VELOCITY], MutatorValue(key.getVelocity()));
	}

	const ShaderProgramResourceVariant* progVariant;
	prog.m_prog->getOrCreateVariant(initInfo, progVariant);

	variant.m_prog = progVariant->getProgram();

	if(!!(RenderingTechniqueBit(1 << key.getRenderingTechnique()) & RenderingTechniqueBit::ALL_RT))
	{
		variant.m_rtShaderGroupHandleIndex = progVariant->getShaderGroupHandleIndex();
	}

	return variant;
}

} // end namespace anki
