// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/MaterialResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ImageResource.h>
#include <AnKi/Core/App.h>
#include <AnKi/Util/Xml.h>

namespace anki {

inline constexpr Array<CString, U32(BuiltinMutatorId::kCount)> kBuiltinMutatorNames = {{"NONE", "ANKI_BONES", "ANKI_VELOCITY"}};

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
#include <AnKi/Gr/ShaderVariableDataType.def.h>
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

static Bool mutatorValueExists(const ShaderBinaryMutator& m, MutatorValue val)
{
	for(MutatorValue v : m.m_values)
	{
		if(v == val)
		{
			return true;
		}
	}

	return false;
}

MaterialVariable::MaterialVariable()
{
	m_Mat4 = Mat4::getZero();
}

MaterialVariable::~MaterialVariable()
{
}

MaterialResource::MaterialResource()
{
}

MaterialResource::~MaterialResource()
{
	ResourceMemoryPool::getSingleton().free(m_prefilledLocalConstants);
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
	ResourceXmlDocument doc;
	XmlElement el;
	ANKI_CHECK(openFileParseXml(filename, doc));

	// <material>
	XmlElement rootEl;
	ANKI_CHECK(doc.getChildElement("material", rootEl));

	// <shaderPrograms>
	XmlElement shaderProgramEl;
	ANKI_CHECK(rootEl.getChildElement("shaderProgram", shaderProgramEl));
	ANKI_CHECK(parseShaderProgram(shaderProgramEl, async));

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

	if(varsSet.getSetBitCount() != m_vars.getSize())
	{
		ANKI_RESOURCE_LOGV("Material doesn't contain default value for %u input variables", U32(m_vars.getSize() - varsSet.getSetBitCount()));

		// Remove unreferenced variables
		ResourceDynamicArray<MaterialVariable> newVars;
		for(U32 i = 0; i < m_vars.getSize(); ++i)
		{
			if(varsSet.get(i))
			{
				newVars.emplaceBack(std::move(m_vars[i]));
			}
		}

		m_vars = std::move(newVars);
	}

	prefillLocalConstants();

	return Error::kNone;
}

Error MaterialResource::parseShaderProgram(XmlElement shaderProgramEl, Bool async)
{
	// name
	CString shaderName;
	ANKI_CHECK(shaderProgramEl.getAttributeText("name", shaderName));

	ResourceString fname;
	fname.sprintf("ShaderBinaries/%s.ankiprogbin", shaderName.cstr());

	ANKI_CHECK(ResourceManager::getSingleton().loadResource(fname, m_prog, async));

	// Find present techniques
	for(const ShaderBinaryTechnique& t : m_prog->getBinary().m_techniques)
	{
		if(t.m_name.getBegin() == CString("GBufferLegacy"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kGBuffer;
			m_shaderTechniques |= ShaderTechniqueBit::kLegacy;
		}
		else if(t.m_name.getBegin() == CString("GBufferMeshShaders"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kGBuffer;
			m_shaderTechniques |= ShaderTechniqueBit::kMeshSaders;
		}
		else if(t.m_name.getBegin() == CString("GBufferSwMeshletRendering"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kGBuffer;
			m_shaderTechniques |= ShaderTechniqueBit::kSwMeshletRendering;
		}
		else if(t.m_name.getBegin() == CString("ShadowsLegacy"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kDepth;
			m_shaderTechniques |= ShaderTechniqueBit::kLegacy;
		}
		else if(t.m_name.getBegin() == CString("ShadowsMeshShaders"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kDepth;
			m_shaderTechniques |= ShaderTechniqueBit::kMeshSaders;
		}
		else if(t.m_name.getBegin() == CString("ShadowsSwMeshletRendering"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kDepth;
			m_shaderTechniques |= ShaderTechniqueBit::kSwMeshletRendering;
		}
		else if(t.m_name.getBegin() == CString("RtShadows"))
		{
			if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
			{
				m_techniquesMask |= RenderingTechniqueBit::kRtShadow;
			}
		}
		else if(t.m_name.getBegin() == CString("Forward"))
		{
			m_techniquesMask |= RenderingTechniqueBit::kForward;
			m_shaderTechniques |= ShaderTechniqueBit::kLegacy;
		}
		else if(t.m_name.getBegin() == CString("RtMaterialFetch"))
		{
			if(GrManager::getSingleton().getDeviceCapabilities().m_rayTracingEnabled)
			{
				m_techniquesMask |= RenderingTechniqueBit::kRtMaterialFetch;
			}
		}
		else
		{
			ANKI_RESOURCE_LOGE("Found unneeded technique in the shader: %s", t.m_name.getBegin());
			return Error::kUserData;
		}
	}

	// <mutation>
	XmlElement mutatorsEl;
	ANKI_CHECK(shaderProgramEl.getChildElementOptional("mutation", mutatorsEl));
	if(mutatorsEl)
	{
		ANKI_CHECK(parseMutators(mutatorsEl));
	}

	// And find the builtin mutators
	ANKI_CHECK(findBuiltinMutators());

	// Create the vars
	ANKI_CHECK(createVars());

	return Error::kNone;
}

Error MaterialResource::parseMutators(XmlElement mutatorsEl)
{
	XmlElement mutatorEl;
	ANKI_CHECK(mutatorsEl.getChildElement("mutator", mutatorEl));

	U32 mutatorCount = 0;
	ANKI_CHECK(mutatorEl.getSiblingElementsCount(mutatorCount));
	++mutatorCount;
	ANKI_ASSERT(mutatorCount > 0);
	m_partialMutation.resize(mutatorCount);
	mutatorCount = 0;

	do
	{
		PartialMutation& pmutation = m_partialMutation[mutatorCount];

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
		pmutation.m_mutator = m_prog->tryFindMutator(mutatorName);

		if(!pmutation.m_mutator)
		{
			ANKI_RESOURCE_LOGE("Mutator not found in program %s", &mutatorName[0]);
			return Error::kUserData;
		}

		if(!mutatorValueExists(*pmutation.m_mutator, pmutation.m_value))
		{
			ANKI_RESOURCE_LOGE("Value %d is not part of the mutator %s", pmutation.m_value, mutatorName.cstr());
			return Error::kUserData;
		}

		// Advance
		++mutatorCount;
		ANKI_CHECK(mutatorEl.getNextSiblingElement("mutator", mutatorEl));
	} while(mutatorEl);

	ANKI_ASSERT(mutatorCount == m_partialMutation.getSize());

	return Error::kNone;
}

Error MaterialResource::findBuiltinMutators()
{
	U builtinMutatorCount = 0;

	// ANKI_BONES
	CString bonesMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kBones];
	const ShaderBinaryMutator* bonesMutator = m_prog->tryFindMutator(bonesMutatorName);
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
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected", bonesMutatorName.cstr());
				return Error::kUserData;
			}
		}

		++builtinMutatorCount;

		m_supportsSkinning = true;
		m_presentBuildinMutatorMask |= U32(1 << BuiltinMutatorId::kBones);
	}

	// VELOCITY
	CString velocityMutatorName = kBuiltinMutatorNames[BuiltinMutatorId::kVelocity];
	const ShaderBinaryMutator* velocityMutator = m_prog->tryFindMutator(velocityMutatorName);
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
				ANKI_RESOURCE_LOGE("Values of the %s mutator in the program are not the expected", velocityMutatorName.cstr());
				return Error::kUserData;
			}
		}

		++builtinMutatorCount;
		m_presentBuildinMutatorMask |= U32(1 << BuiltinMutatorId::kVelocity);
	}

	if(m_partialMutation.getSize() + builtinMutatorCount != m_prog->getBinary().m_mutators.getSize())
	{
		ANKI_RESOURCE_LOGE("Some mutatators are unacounted for");
		return Error::kUserData;
	}

	return Error::kNone;
}

Error MaterialResource::createVars()
{
	const ShaderBinary& binary = m_prog->getBinary();

	// Find struct
	const ShaderBinaryStruct* localConstantsStruct = nullptr;
	for(const ShaderBinaryStruct& strct : binary.m_structs)
	{
		if(CString(strct.m_name.getBegin()) == "AnKiLocalConstants")
		{
			localConstantsStruct = &strct;
			break;
		}
	}

	// Create vars
	for(U32 i = 0; localConstantsStruct && i < localConstantsStruct->m_members.getSize(); ++i)
	{
		const ShaderBinaryStructMember& member = localConstantsStruct->m_members[i];
		const CString memberName = member.m_name.getBegin();

		MaterialVariable& var = *m_vars.emplaceBack();
		zeroMemory(var);
		var.m_name = memberName;
		var.m_dataType = member.m_type;
		var.m_offsetInLocalConstants = member.m_offset;
	}

	m_localConstantsSize = (localConstantsStruct) ? localConstantsStruct->m_size : 0;

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
	if(foundVar->m_dataType == ShaderVariableDataType::kU32)
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
			ANKI_CHECK(ResourceManager::getSingleton().loadResource(value, foundVar->m_image, async));

			foundVar->m_U32 = foundVar->m_image->getTexture().getOrCreateBindlessTextureIndex(TextureSubresourceDesc::all());
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
#include <AnKi/Gr/ShaderVariableDataType.def.h>
#undef ANKI_SVDT_MACRO
		default:
			ANKI_ASSERT(0);
			break;
		}
	}

	return Error::kNone;
}

void MaterialResource::prefillLocalConstants()
{
	if(m_localConstantsSize == 0)
	{
		return;
	}

	m_prefilledLocalConstants = ResourceMemoryPool::getSingleton().allocate(m_localConstantsSize, 1);
	memset(m_prefilledLocalConstants, 0, m_localConstantsSize);

	for(const MaterialVariable& var : m_vars)
	{
		switch(var.m_dataType)
		{
#define ANKI_SVDT_MACRO(type, baseType, rowCount, columnCount, isIntagralType) \
	case ShaderVariableDataType::k##type: \
		ANKI_ASSERT(var.m_offsetInLocalConstants + sizeof(type) <= m_localConstantsSize); \
		memcpy(static_cast<U8*>(m_prefilledLocalConstants) + var.m_offsetInLocalConstants, &var.m_##type, sizeof(type)); \
		break;
#include <AnKi/Gr/ShaderVariableDataType.def.h>
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

	// Sanitize the key
	if(!(m_presentBuildinMutatorMask & U32(BuiltinMutatorId::kVelocity)) && key.getVelocity())
	{
		// Particles set their own velocity
		key.setVelocity(false);
	}

	if(key.getRenderingTechnique() != RenderingTechnique::kGBuffer && key.getVelocity())
	{
		// Only GBuffer technique can write to velocity buffers
		key.setVelocity(false);
	}

	const Bool meshShadersSupported = GrManager::getSingleton().getDeviceCapabilities().m_meshShaders;
	ANKI_ASSERT(!(key.getMeshletRendering() && (!meshShadersSupported && !g_meshletRenderingCVar))
				&& "Can't be asking for meshlet rendering if mesh shaders or SW meshlet rendering are not supported/enabled");
	if(key.getMeshletRendering() && !(m_shaderTechniques & (ShaderTechniqueBit::kMeshSaders | ShaderTechniqueBit::kSwMeshletRendering)))
	{
		key.setMeshletRendering(false);
	}

	ANKI_ASSERT(!key.getSkinned() || !!(m_presentBuildinMutatorMask & U32(1 << BuiltinMutatorId::kBones)));
	ANKI_ASSERT(!key.getVelocity() || !!(m_presentBuildinMutatorMask & U32(1 << BuiltinMutatorId::kVelocity)));

	MaterialVariant& variant = m_variantMatrix[key.getRenderingTechnique()][key.getSkinned()][key.getVelocity()][key.getMeshletRendering()];

	// Check if it's initialized
	{
		RLockGuard<RWMutex> lock(m_variantMatrixMtx);
		if(variant.m_prog.isCreated()) [[likely]]
		{
			return variant;
		}
	}

	// Not initialized, init it
	WLockGuard<RWMutex> lock(m_variantMatrixMtx);

	// Check again
	if(variant.m_prog.isCreated())
	{
		return variant;
	}

	ShaderProgramResourceVariantInitInfo initInfo(m_prog);

	for(const PartialMutation& m : m_partialMutation)
	{
		initInfo.addMutation(m.m_mutator->m_name.getBegin(), m.m_value);
	}

	if(!!(m_presentBuildinMutatorMask & U32(1 << BuiltinMutatorId::kBones)))
	{
		initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kBones], MutatorValue(key.getSkinned()));
	}

	if(!!(m_presentBuildinMutatorMask & U32(1 << BuiltinMutatorId::kVelocity)))
	{
		initInfo.addMutation(kBuiltinMutatorNames[BuiltinMutatorId::kVelocity], MutatorValue(key.getVelocity()));
	}

	switch(key.getRenderingTechnique())
	{
	case RenderingTechnique::kGBuffer:
		if(key.getMeshletRendering() && meshShadersSupported)
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kMesh | ShaderTypeBit::kPixel, "GBufferMeshShaders");
		}
		else if(key.getMeshletRendering())
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "GBufferSwMeshletRendering");
		}
		else
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "GBufferLegacy");
		}
		break;
	case RenderingTechnique::kDepth:
		if(key.getMeshletRendering() && meshShadersSupported)
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kMesh | ShaderTypeBit::kPixel, "ShadowsMeshShaders");
		}
		else if(key.getMeshletRendering())
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "ShadowsSwMeshletRendering");
		}
		else
		{
			initInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "ShadowsLegacy");
		}
		break;
	case RenderingTechnique::kForward:
		initInfo.requestTechniqueAndTypes(ShaderTypeBit::kVertex | ShaderTypeBit::kPixel, "Forward");
		break;
	case RenderingTechnique::kRtShadow:
		initInfo.requestTechniqueAndTypes(ShaderTypeBit::kAllHit, "RtShadows");
		break;
	case RenderingTechnique::kRtMaterialFetch:
		initInfo.requestTechniqueAndTypes(ShaderTypeBit::kAllHit, "RtMaterialFetch");
		break;
	default:
		ANKI_ASSERT(0);
	}

	const ShaderProgramResourceVariant* progVariant = nullptr;
	m_prog->getOrCreateVariant(initInfo, progVariant);

	if(!progVariant)
	{
		ANKI_RESOURCE_LOGF("Fetched skipped mutation on program %s", getFilename().cstr());
	}

	variant.m_prog.reset(&progVariant->getProgram());

	if(!!(RenderingTechniqueBit(1 << key.getRenderingTechnique()) & RenderingTechniqueBit::kAllRt))
	{
		variant.m_rtShaderGroupHandleIndex = progVariant->getShaderGroupHandleIndex();
	}

	return variant;
}

} // end namespace anki
