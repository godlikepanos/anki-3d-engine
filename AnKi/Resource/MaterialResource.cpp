// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Util/Xml.h>

namespace anki {

inline constexpr Array<CString, U32(BuiltinMutatorId::kCount)> kBuiltinMutatorNames = {
	{"NONE", "ANKI_TECHNIQUE", "ANKI_LOD", "ANKI_BONES", "ANKI_VELOCITY"}};

inline constexpr Array<CString, U(RenderingTechnique::kCount)> kTechniqueNames = {
	{"GBuffer", "GBufferEarlyZ", "Shadow", "Forward", "RtShadow"}};

// This is some trickery to select calling between XmlElement::getAttributeNumber and XmlElement::getAttributeNumbers
namespace {

template<typename T>
class IsShaderVarDataTypeAnArray
{
public:
	static constexpr Bool kValue = false;
};

#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	template<> \
	class IsShaderVarDataTypeAnArray<type> \
	{ \
	public: \
		static constexpr Bool kValue = rowCount * columnCount > 1; \
	};
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO

template<typename T, Bool isArray = IsShaderVarDataTypeAnArray<T>::kValue>
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

class MaterialResource::Program
{
public:
	ShaderProgramResourcePtr m_prog;

	mutable Array4d<MaterialVariant, U(RenderingTechnique::kCount), kMaxLodCount, 2, 2> m_variantMatrix;
	mutable RWMutex m_variantMatrixMtx;

	DynamicArray<PartialMutation> m_partialMutation; ///< Only with the non-builtins.

	U32 m_presentBuildinMutators = 0;
	U32 m_localUniformsStructIdx = 0; ///< Struct index in the program binary.

	U8 m_lodCount = 1;

	Program() = default;

	Program(const Program&) = delete; // Non-copyable

	Program(Program&& b)
	{
		*this = std::move(b);
	}

	Program& operator=(const Program& b) = delete; // Non-copyable

	Program& operator=(Program&& b)
	{
		m_prog = std::move(b.m_prog);
		for(RenderingTechnique t : EnumIterable<RenderingTechnique>())
		{
			for(U32 l = 0; l < kMaxLodCount; ++l)
			{
				for(U32 skin = 0; skin < 2; ++skin)
				{
					for(U32 vel = 0; vel < 2; ++vel)
					{
						m_variantMatrix[t][skin][skin][vel] = std::move(b.m_variantMatrix[t][skin][skin][vel]);
					}
				}
			}
		}
		m_partialMutation = std::move(b.m_partialMutation);
		m_presentBuildinMutators = b.m_presentBuildinMutators;
		m_localUniformsStructIdx = b.m_localUniformsStructIdx;
		m_lodCount = b.m_lodCount;
		return *this;
	}
};

MaterialResource::MaterialResource(ResourceManager* manager)
	: ResourceObject(manager)
{
	memset(m_techniqueToProgram.getBegin(), 0xFF, m_techniqueToProgram.getSizeInBytes());
}

MaterialResource::~MaterialResource()
{
	for(Program& p : m_programs)
	{
		p.m_partialMutation.destroy(getMemoryPool());
	}

	m_textures.destroy(getMemoryPool());

	for(MaterialVariable& var : m_vars)
	{
		var.m_name.destroy(getMemoryPool());
	}

	m_vars.destroy(getMemoryPool());
	m_programs.destroy(getMemoryPool());

	getMemoryPool().free(m_prefilledLocalUniforms);
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
	XmlDocument doc(&getTempMemoryPool());
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
		return Error::kUserData;
	}

	prefillLocalUniforms();

	return Error::kNone;
}

Error MaterialResource::parseShaderProgram(XmlElement shaderProgramEl, Bool async)
{
	// name
	CString shaderName;
	ANKI_CHECK(shaderProgramEl.getAttributeText("name", shaderName));

	if(!getManager().getGrManager().getDeviceCapabilities().m_rayTracingEnabled && shaderName.find("Rt") == 0)
	{
		// Skip RT programs when RT is disabled
		return Error::kNone;
	}

	StringRaii fname(&getTempMemoryPool());
	fname.sprintf("ShaderBinaries/%s.ankiprogbin", shaderName.cstr());

	Program& prog = *m_programs.emplaceBack(getMemoryPool());
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

	return Error::kNone;
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
		prog.m_localUniformsStructIdx = kMaxU32;
	}

	// Iterate all members of the local uniforms struct to add its members
	U32 offsetof = 0;
	for(U32 i = 0; localUniformsStruct && i < localUniformsStruct->m_members.getSize(); ++i)
	{
		const ShaderProgramBinaryStructMember& member = localUniformsStruct->m_members[i];
		const CString memberName = member.m_name.getBegin();

		// Check if it needs to be added
		Bool addIt = false;
		if(member.m_dependentMutator == kMaxU32)
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
				return Error::kUserData;
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
					return Error::kUserData;
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
						return Error::kUserData;
					}
				}

				// All good, add it
				var = m_vars.emplaceBack(getMemoryPool());
				var->m_name.create(getMemoryPool(), memberName);
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

	Array<const ShaderProgramResourceMutator*, U(BuiltinMutatorId::kCount)> mutatorPtrs = {};
	for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
	{
		mutatorPtrs[id] = prog.m_prog->tryFindMutator(kBuiltinMutatorNames[id]);
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

	ANKI_LOOP(kTechnique)
	ANKI_LOOP(kLod)
	ANKI_LOOP(kBones)
	ANKI_LOOP(kVelocity)
	{
		const ShaderProgramResourceVariant* variant;
		prog.m_prog->getOrCreateVariant(initInfo, variant);
		if(!variant)
		{
			// Skipped variant
			continue;
		}

		// Add opaque vars
		for(const ShaderProgramBinaryOpaqueInstance& instance : variant->getBinaryVariant().m_opaques)
		{
			const ShaderProgramBinaryOpaque& opaque = binary.m_opaques[instance.m_index];
			if(opaque.m_type == ShaderVariableDataType::kSampler)
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
					return Error::kUserData;
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
						return Error::kUserData;
					}
				}

				// All good, add it
				var = m_vars.emplaceBack(getMemoryPool());
				var->m_name.create(getMemoryPool(), opaqueName);
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

	return Error::kNone;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl, Program& prog)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	prog.m_partialMutation.create(getMemoryPool(), mutatorCount);
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
			return Error::kUserData;
		}

		for(BuiltinMutatorId id : EnumIterable<BuiltinMutatorId>())
		{
			if(id == BuiltinMutatorId::kNone)
			{
				continue;
			}

			if(mutatorName == kBuiltinMutatorNames[id])
			{
				ANKI_RESOURCE_LOGE("Materials shouldn't list builtin mutators: %s", mutatorName.cstr());
				return Error::kUserData;
			}
		}

		if(mutatorName.find("ANKI_") == 0)
		{
			ANKI_RESOURCE_LOGE("Mutators can't start with ANKI_: %s", mutatorName.cstr());
			return Error::kUserData;
		}

		// value
		ANKI_CHECK(mutatorEl.getAttributeNumber("value", pmutation.m_value));

		// Find mutator
		pmutation.m_mutator = prog.m_prog->tryFindMutator(mutatorName);

		if(!pmutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::kUserData;
		}

		if(!pmutation.m_mutator->valueExists(pmutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", pmutation.m_value, mutatorName.cstr());
			return Error::kUserData;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	ANKI_ASSERT(mutatorCount == prog.m_partialMutation.getSize());

	return Error::kNone;
}

Error MaterialResource::findBuiltinMutators(Program& prog)
{
	U builtinMutatorCount = 0;

	// ANKI_TECHNIQUE
	CString techniqueMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kTechnique];
	const ShaderProgramResourceMutator* techniqueMutator = prog.m_prog->tryFindMutator(techniqueMutatorName);
	if(techniqueMutator)
	{
		for(U32 i = 0; i < techniqueMutator->m_values.getSize(); ++i)
		{
			const MutatorValue mvalue = techniqueMutator->m_values[i];
			if(mvalue >= MutatorValue(RenderingTechnique::kCount) || mvalue < MutatorValue(RenderingTechnique::kFirst))
			{
				ANKI_RESOURCE_LOGE("Mutator %s has a wrong value %d", techniqueMutatorName.cstr(), mvalue);
				return Error::kUserData;
			}

			const RenderingTechnique techniqueId = RenderingTechnique(mvalue);
			const PtrSize progIdx = &prog - m_programs.getBegin();
			ANKI_ASSERT(progIdx < m_programs.getSize());
			m_techniqueToProgram[techniqueId] = U8(progIdx);

			const RenderingTechniqueBit mask = RenderingTechniqueBit(1 << techniqueId);
			if(!!(m_techniquesMask & mask))
			{
				ANKI_RESOURCE_LOGE("The %s technique appeared more than once", kTechniqueNames[mvalue].cstr());
				return Error::kUserData;
			}
			m_techniquesMask |= mask;
		}

		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::kTechnique);
	}
	else
	{
		ANKI_RESOURCE_LOGE("Mutator %s should be present in every shader program referenced by a material",
						   techniqueMutatorName.cstr());
		return Error::kUserData;
	}

	// ANKI_LOD
	CString lodMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kLod];
	const ShaderProgramResourceMutator* lodMutator = prog.m_prog->tryFindMutator(lodMutatorName);
	if(lodMutator)
	{
		if(lodMutator->m_values.getSize() > kMaxLodCount)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have at least %u values in the program", lodMutatorName.cstr(),
							   U32(kMaxLodCount));
			return Error::kUserData;
		}

		for(U32 i = 0; i < lodMutator->m_values.getSize(); ++i)
		{
			if(lodMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   lodMutatorName.cstr());
				return Error::kUserData;
			}
		}

		prog.m_lodCount = U8(lodMutator->m_values.getSize());
		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::kLod);
	}

	// ANKI_BONES
	CString bonesMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kBones];
	const ShaderProgramResourceMutator* bonesMutator = prog.m_prog->tryFindMutator(bonesMutatorName);
	if(bonesMutator)
	{
		if(bonesMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", bonesMutatorName.cstr());
			return Error::kUserData;
		}

		for(U32 i = 0; i < bonesMutator->m_values.getSize(); ++i)
		{
			if(bonesMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   bonesMutatorName.cstr());
				return Error::kUserData;
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
			if((binding == kMaterialBindingBoneTransforms || binding == kMaterialBindingPreviousBoneTransforms)
			   && set == kMaterialSetLocal)
			{
				++foundCount;
			}
		}

		if(foundCount != 2)
		{
			ANKI_RESOURCE_LOGE("The shader has %s mutation but not the storage buffers for the bone transforms",
							   bonesMutatorName.cstr());
			return Error::kUserData;
		}

		m_supportsSkinning = true;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::kBones);
	}

	// VELOCITY
	CString velocityMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kVelocity];
	const ShaderProgramResourceMutator* velocityMutator = prog.m_prog->tryFindMutator(velocityMutatorName);
	if(velocityMutator)
	{
		if(velocityMutator->m_values.getSize() != 2)
		{
			ANKI_RESOURCE_LOGE("Mutator %s should have 2 values in the program", velocityMutatorName.cstr());
			return Error::kUserData;
		}

		for(U32 i = 0; i < velocityMutator->m_values.getSize(); ++i)
		{
			if(velocityMutator->m_values[i] != MutatorValue(i))
			{
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected",
								   velocityMutatorName.cstr());
				return Error::kUserData;
			}
		}

		++builtinMutatorCount;
		prog.m_presentBuildinMutators |= U32(1 << BuiltinMutatorId::kVelocity);
	}

	if(prog.m_partialMutation.getSize() + builtinMutatorCount != prog.m_prog->getMutators().getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::kUserData;
	}

	return Error::kNone;
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
		return Error::kUserData;
	}

	// Set check
	const U32 idx = U32(foundVar - m_vars.getBegin());
	if(varsSet.get(idx))
	{
		ANKI_RESOURCE_LOGE("Input already has a value: %s", varName.cstr());
		return Error::kUserData;
	}
	varsSet.set(idx);

	// Set the value
	if(foundVar->isBoundableTexture())
	{
		CString texfname;
		ANKI_CHECK(inputEl.getAttributeText("value", texfname));
		ANKI_CHECK(getManager().loadResource(texfname, foundVar->m_image, async));

		m_textures.emplaceBack(getMemoryPool(), foundVar->m_image->getTexture());
	}
	else if(foundVar->m_dataType == ShaderVariableDataType::kU32)
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
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
		ANKI_CHECK(GetAttribute<type>()(inputEl, foundVar->ANKI_CONCATENATE(m_, type))); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.defs.h>
#undef ANKI_SVDT_MACRO
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	return Error::kNone;
}

void MaterialResource::prefillLocalUniforms()
{
	if(m_localUniformsSize == 0)
	{
		return;
	}

	m_prefilledLocalUniforms = getMemoryPool().allocate(m_localUniformsSize, 1);
	memset(m_prefilledLocalUniforms, 0, m_localUniformsSize);

	for(const MaterialVariable& var : m_vars)
	{
		if(!var.isUniform())
		{
			continue;
		}

		switch(var.m_dataType)
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
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
	ANKI_ASSERT(m_techniqueToProgram[key.getRenderingTechnique()] != kMaxU8);
	const Program& prog = m_programs[m_techniqueToProgram[key.getRenderingTechnique()]];

	// Sanitize the key
	key.setLod(min<U32>(prog.m_lodCount - 1, key.getLod()));

	if(key.getRenderingTechnique() == RenderingTechnique::kGBufferEarlyZ
	   || key.getRenderingTechnique() == RenderingTechnique::kShadow)
	{
		key.setLod(0);
	}

	if(!(prog.m_presentBuildinMutators & U32(BuiltinMutatorId::kVelocity)) && key.getVelocity())
	{
		// Particles set their own velocity
		key.setVelocity(false);
	}

	ANKI_ASSERT(!key.getSkinned() || !!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::kBones)));
	ANKI_ASSERT(!key.getVelocity() || !!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::kVelocity)));

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

	initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kTechnique], MutatorValue(key.getRenderingTechnique()));

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::kLod)))
	{
		initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kLod], MutatorValue(key.getLod()));
	}

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::kBones)))
	{
		initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kBones], MutatorValue(key.getSkinned()));
	}

	if(!!(prog.m_presentBuildinMutators & U32(1 << BuiltinMutatorId::kVelocity)))
	{
		initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kVelocity], MutatorValue(key.getVelocity()));
	}

	const ShaderProgramResourceVariant* progVariant;
	prog.m_prog->getOrCreateVariant(initInfo, progVariant);

	if(!progVariant)
	{
		ANKI_RESOURCE_LOGF("Fetched skipped mutation on program %s", getFilename().cstr());
	}

	variant.m_prog = progVariant->getProgram();

	if(!!(RenderingTechniqueBit(1 << key.getRenderingTechnique()) & RenderingTechniqueBit::kAllRt))
	{
		variant.m_rtShaderGroupHandleIndex = progVariant->getShaderGroupHandleIndex();
	}

	return variant;
}

} // end namespace anki
