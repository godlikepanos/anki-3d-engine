// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ShaderProgramResource.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Gr/ShaderProgram.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Core/CVarSet.h>

namespace anki {

ShaderProgramResourceVariant::ShaderProgramResourceVariant()
{
}

ShaderProgramResourceVariant::~ShaderProgramResourceVariant()
{
}

ShaderProgramResource::ShaderProgramResource()
{
}

ShaderProgramResource::~ShaderProgramResource()
{
	for(auto it : m_variants)
	{
		ShaderProgramResourceVariant* variant = &(*it);
		deleteInstance(ResourceMemoryPool::getSingleton(), variant);
	}

	ResourceMemoryPool::getSingleton().free(m_binary);
}

Error ShaderProgramResource::load(const ResourceFilename& filename, [[maybe_unused]] Bool async)
{
	// Load the binary
	ResourceFilePtr file;
	ANKI_CHECK(openFile(filename, file));
	ANKI_CHECK(deserializeShaderProgramBinaryFromAnyFile(*file, m_binary, ResourceMemoryPool::getSingleton()));

	return Error::kNone;
}

void ShaderProgramResource::getOrCreateVariant(const ShaderProgramResourceVariantInitInfo& info_, const ShaderProgramResourceVariant*& variant) const
{
	ShaderProgramResourceVariantInitInfo info = info_;

	// Sanity checks
	ANKI_ASSERT(info.m_setMutators.getSetBitCount() == m_binary->m_mutators.getSize());

	// Find the technique
	const CString techniqueName = info.m_techniqueName.getBegin();
	U32 techniqueIdx = kMaxU32;
	for(U32 i = 0; i < m_binary->m_techniques.getSize(); ++i)
	{
		if(m_binary->m_techniques[i].m_name.getBegin() == techniqueName)
		{
			techniqueIdx = i;
			break;
		}
	}
	ANKI_ASSERT(techniqueIdx != kMaxU32 && "Technique not found");
	const ShaderProgramBinaryTechnique& technique = m_binary->m_techniques[techniqueIdx];

	// Find the shader stages
	if(info.m_shaderTypes == ShaderTypeBit::kNone)
	{
		// Try to guess the shader types

		if(technique.m_shaderTypes == ShaderTypeBit::kCompute)
		{
			// Only compute
		}
		else if((technique.m_shaderTypes & ~(ShaderTypeBit::kAllLegacyGeometry | ShaderTypeBit::kFragment)) == ShaderTypeBit::kNone)
		{
			// Only legacy geometry
		}
		else if((technique.m_shaderTypes & ~(ShaderTypeBit::kAllModernGeometry | ShaderTypeBit::kFragment)) == ShaderTypeBit::kNone)
		{
			// Only modern geometry
		}
		else if((technique.m_shaderTypes & ~ShaderTypeBit::kAllHit) == ShaderTypeBit::kNone)
		{
			// Only hit
		}
		else if(technique.m_shaderTypes == ShaderTypeBit::kMiss)
		{
			// Only miss
		}
		else if(technique.m_shaderTypes == ShaderTypeBit::kRayGen)
		{
			// Only ray gen
		}
		else
		{
			ANKI_ASSERT(!"Can't figure out a sensible default");
		}

		info.m_shaderTypes = technique.m_shaderTypes;
	}

	ANKI_ASSERT((info.m_shaderTypes & technique.m_shaderTypes) == info.m_shaderTypes);

	// Compute variant hash
	U64 hash = computeHash(&info.m_shaderTypes, sizeof(info.m_shaderTypes));
	hash = appendHash(techniqueName.cstr(), techniqueName.getLength(), hash);
	if(m_binary->m_mutators.getSize())
	{
		hash = appendHash(info.m_mutation.getBegin(), m_binary->m_mutators.getSize() * sizeof(info.m_mutation[0]), hash);
	}

	// Check if the variant is in the cache
	{
		RLockGuard<RWMutex> lock(m_mtx);

		auto it = m_variants.find(hash);
		if(it != m_variants.getEnd())
		{
			// Done
			variant = *it;
			if(!!(info.m_shaderTypes & ShaderTypeBit::kAllGraphics))
			{
				ANKI_ASSERT(variant->m_prog->getShaderTypes() == info.m_shaderTypes);
			}
			return;
		}
	}

	// Create the variant
	WLockGuard<RWMutex> lock(m_mtx);

	// Check again
	auto it = m_variants.find(hash);
	if(it != m_variants.getEnd())
	{
		// Done
		variant = *it;
		return;
	}

	// Create
	ShaderProgramResourceVariant* v = createNewVariant(info, techniqueIdx);
	if(v)
	{
		m_variants.emplace(hash, v);
	}
	variant = v;
	if(!!(info.m_shaderTypes & ShaderTypeBit::kAllGraphics))
	{
		ANKI_ASSERT(variant->m_prog->getShaderTypes() == info.m_shaderTypes);
	}
}

ShaderProgramResourceVariant* ShaderProgramResource::createNewVariant(const ShaderProgramResourceVariantInitInfo& info, U32 techniqueIdx) const
{
	// Get the binary program variant
	const ShaderProgramBinaryVariant* binaryVariant = nullptr;
	U64 mutationHash = 0;
	if(m_binary->m_mutators.getSize())
	{
		// Create the mutation hash
		mutationHash = computeHash(info.m_mutation.getBegin(), m_binary->m_mutators.getSize() * sizeof(info.m_mutation[0]));

		// Search for the mutation in the binary
		// TODO optimize the search
		for(const ShaderProgramBinaryMutation& mutation : m_binary->m_mutations)
		{
			if(mutation.m_hash == mutationHash)
			{
				if(mutation.m_variantIndex == kMaxU32)
				{
					// Skipped mutation, nothing to create
					return nullptr;
				}

				binaryVariant = &m_binary->m_variants[mutation.m_variantIndex];
				break;
			}
		}
	}
	else
	{
		ANKI_ASSERT(m_binary->m_variants.getSize() == 1);
		binaryVariant = &m_binary->m_variants[0];
	}
	ANKI_ASSERT(binaryVariant);
	ShaderProgramResourceVariant* variant = newInstance<ShaderProgramResourceVariant>(ResourceMemoryPool::getSingleton());

	// Time to init the shaders
	if(!!(info.m_shaderTypes & (ShaderTypeBit::kAllGraphics | ShaderTypeBit::kCompute)))
	{
		// Create the program name
		String progName;
		getFilepathFilename(getFilename(), progName);

		ShaderProgramInitInfo progInf(progName);
		Array<ShaderPtr, U32(ShaderType::kCount)> shaderRefs; // Just for refcounting
		for(ShaderType shaderType : EnumBitsIterable<ShaderType, ShaderTypeBit>(info.m_shaderTypes))
		{
			const ResourceString shaderName = (progName + "_" + m_binary->m_techniques[techniqueIdx].m_name.getBegin()).cstr();
			ShaderInitInfo inf(shaderName);
			inf.m_shaderType = shaderType;
			inf.m_binary = m_binary->m_codeBlocks[binaryVariant->m_techniqueCodeBlocks[techniqueIdx].m_codeBlockIndices[shaderType]].m_binary;
			ShaderPtr shader = GrManager::getSingleton().newShader(inf);
			shaderRefs[shaderType] = shader;

			const ShaderTypeBit shaderBit = ShaderTypeBit(1 << shaderType);
			if(!!(shaderBit & ShaderTypeBit::kAllGraphics))
			{
				progInf.m_graphicsShaders[shaderType] = shader.get();
			}
			else if(shaderType == ShaderType::kCompute)
			{
				progInf.m_computeShader = shader.get();
			}
			else
			{
				ANKI_ASSERT(0);
			}
		}

		// Create the program
		variant->m_prog = GrManager::getSingleton().newShaderProgram(progInf);
	}
	else
	{
		ANKI_ASSERT(!!(info.m_shaderTypes & ShaderTypeBit::kAllRayTracing));

		// Find the library
		const CString libName = m_binary->m_techniques[techniqueIdx].m_name.getBegin();
		ANKI_ASSERT(libName.getLength() > 0);

		const ShaderProgramResourceSystem& progSystem = ResourceManager::getSingleton().getShaderProgramResourceSystem();
		const ShaderProgramRaytracingLibrary* foundLib = nullptr;
		for(const ShaderProgramRaytracingLibrary& lib : progSystem.getRayTracingLibraries())
		{
			if(lib.getLibraryName() == libName)
			{
				foundLib = &lib;
				break;
			}
		}
		ANKI_ASSERT(foundLib);

		variant->m_prog = foundLib->getShaderProgram();

		RayTracingShaderGroupType groupType;
		if(!!(info.m_shaderTypes & ShaderTypeBit::kRayGen))
		{
			groupType = RayTracingShaderGroupType::kRayGen;
		}
		else if(!!(info.m_shaderTypes & ShaderTypeBit::kMiss))
		{
			groupType = RayTracingShaderGroupType::kMiss;
		}
		else
		{
			ANKI_ASSERT(!!(info.m_shaderTypes & ShaderTypeBit::kAllHit));
			groupType = RayTracingShaderGroupType::kHit;
		}

		// Set the group handle index
		variant->m_shaderGroupHandleIndex = foundLib->getShaderGroupHandleIndex(getFilename(), mutationHash, groupType);
	}

	return variant;
}

} // end namespace anki
