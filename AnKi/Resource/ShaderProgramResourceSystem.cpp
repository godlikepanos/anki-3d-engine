// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Resource/ResourceManager.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/CommandBuffer.h>
#include <AnKi/Gr/Fence.h>
#include <AnKi/ShaderCompiler/ShaderCompiler.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>

namespace anki {

class ShaderProgramResourceSystem::ShaderH
{
public:
	ShaderPtr m_shader;
	U64 m_hash = 0; ///< Hash of the binary.
};

class ShaderProgramResourceSystem::ShaderGroup
{
public:
	U32 m_rayGen = kMaxU32;
	U32 m_miss = kMaxU32;
	U32 m_chit = kMaxU32;
	U32 m_ahit = kMaxU32;

	ResourceDynamicArray<U64> m_groupHashes;
};

class ShaderProgramResourceSystem::Lib
{
public:
	ResourceString m_name;
	ResourceDynamicArray<ShaderH> m_shaders;
	ResourceDynamicArray<ShaderGroup> m_rayGenGroups;
	ResourceDynamicArray<ShaderGroup> m_missGroups;
	ResourceDynamicArray<ShaderGroup> m_hitGroups;
	ShaderTypeBit m_presentStages = ShaderTypeBit::kNone;

	U32 addShader(const ShaderBinaryCodeBlock& codeBlock, CString progName, ShaderType shaderType)
	{
		ShaderH* shader = nullptr;
		for(ShaderH& s : m_shaders)
		{
			if(s.m_hash == codeBlock.m_hash)
			{
				shader = &s;
				break;
			}
		}

		if(shader == nullptr)
		{
			shader = m_shaders.emplaceBack();

			ShaderInitInfo inf(progName);
			inf.m_shaderType = shaderType;
			inf.m_binary = codeBlock.m_binary;
			inf.m_reflection = codeBlock.m_reflection;
			shader->m_shader = GrManager::getSingleton().newShader(inf);
			shader->m_hash = codeBlock.m_hash;

			m_presentStages |= ShaderTypeBit(1 << shaderType);
		}

		return U32(shader - m_shaders.getBegin());
	}

	void addGroup(CString filename, U64 mutationHash, U32 rayGen, U32 miss, U32 chit, U32 ahit)
	{
		ResourceDynamicArray<ShaderGroup>* groupArray = nullptr;
		RayTracingShaderGroupType groupType;
		if(rayGen < kMaxU32)
		{
			groupType = RayTracingShaderGroupType::kRayGen;
			groupArray = &m_rayGenGroups;
		}
		else if(miss < kMaxU32)
		{
			groupType = RayTracingShaderGroupType::kMiss;
			groupArray = &m_missGroups;
		}
		else
		{
			ANKI_ASSERT(chit < kMaxU32 || ahit < kMaxU32);
			groupType = RayTracingShaderGroupType::kHit;
			groupArray = &m_hitGroups;
		}

		ShaderGroup* pgroup = nullptr;
		for(ShaderGroup& s : *groupArray)
		{
			if(s.m_rayGen == rayGen && s.m_miss == miss && s.m_ahit == ahit && s.m_chit == chit)
			{
				pgroup = &s;
				break;
			}
		}

		if(!pgroup)
		{
			pgroup = groupArray->emplaceBack();
			pgroup->m_rayGen = rayGen;
			pgroup->m_miss = miss;
			pgroup->m_chit = chit;
			pgroup->m_ahit = ahit;
		}

		const U64 groupHash = ShaderProgramRaytracingLibrary::generateShaderGroupGroupHash(filename, mutationHash, groupType);
#if ANKI_ASSERTIONS_ENABLED
		for(U64 hash : pgroup->m_groupHashes)
		{
			ANKI_ASSERT(hash != groupHash && "Shouldn't find group with the same hash");
		}
#endif

		pgroup->m_groupHashes.emplaceBack(groupHash);
	}
};

U64 ShaderProgramRaytracingLibrary::generateShaderGroupGroupHash(CString resourceFilename, U64 mutationHash, RayTracingShaderGroupType groupType)
{
	ANKI_ASSERT(resourceFilename.getLength() > 0);
	ANKI_ASSERT(groupType < RayTracingShaderGroupType::kCount);
	const String basename = getFilename(resourceFilename);
	U64 hash = appendHash(basename.cstr(), basename.getLength(), mutationHash);
	hash = appendHash(&groupType, sizeof(groupType), hash);
	return hash;
}

Error ShaderProgramResourceSystem::init()
{
	if(!GrManager::getSingleton().getDeviceCapabilities().m_rayTracing)
	{
		return Error::kNone;
	}

	// Create RT pipeline libraries
	const Error err = createRayTracingPrograms(m_rtLibraries);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to create ray tracing programs");
	}

	return err;
}

Error ShaderProgramResourceSystem::createRayTracingPrograms(ResourceDynamicArray<ShaderProgramRaytracingLibrary>& outLibs)
{
	ANKI_RESOURCE_LOGI("Creating ray tracing programs");
	U32 rtProgramCount = 0;

	ResourceDynamicArray<Lib> libs;

	Error err = Error::kNone;
	ResourceFilesystem::getSingleton().iterateAllFilenames([&](CString filename) {
		// Check file extension
		const String extension = getFileExtension(filename);
		const Char binExtension[] = "ankiprogbin";
		if(extension.getLength() != sizeof(binExtension) - 1 || extension != binExtension)
		{
			return FunctorContinue::kContinue;
		}

		// Get the binary
		ResourceFilePtr file;
		if((err = ResourceFilesystem::getSingleton().openFile(filename, file)))
		{
			return FunctorContinue::kStop;
		}
		ShaderBinary* binary;
		if((err = deserializeShaderBinaryFromAnyFile(*file, binary, ResourceMemoryPool::getSingleton())))
		{
			return FunctorContinue::kStop;
		}

		class Dummy
		{
		public:
			ShaderBinary* m_binary;

			~Dummy()
			{
				ResourceMemoryPool::getSingleton().free(m_binary);
			}
		} dummy{binary};

		if(!(binary->m_shaderTypes & ShaderTypeBit::kAllRayTracing))
		{
			return FunctorContinue::kContinue;
		}

		// Create the program name
		const String progName = getFilename(filename);

		for(const ShaderBinaryTechnique& technique : binary->m_techniques)
		{
			if(!(technique.m_shaderTypes & ShaderTypeBit::kAllRayTracing))
			{
				continue;
			}

			const U32 techniqueIdx = U32(&technique - binary->m_techniques.getBegin());

			// Find or create the lib
			Lib* lib = nullptr;
			{
				for(Lib& l : libs)
				{
					if(l.m_name == technique.m_name.getBegin())
					{
						lib = &l;
						break;
					}
				}

				if(lib == nullptr)
				{
					libs.emplaceBack();
					lib = &libs.getBack();
					lib->m_name = technique.m_name.getBegin();
				}
			}

			// Ray gen
			if(!!(technique.m_shaderTypes & ShaderTypeBit::kRayGen))
			{
				// Iterate all mutations
				ConstWeakArray<ShaderBinaryMutation> mutations;
				ShaderBinaryMutation dummyMutation;
				if(binary->m_mutations.getSize() > 1)
				{
					mutations = binary->m_mutations;
				}
				else
				{
					dummyMutation.m_hash = 0;
					dummyMutation.m_variantIndex = 0;
					mutations = ConstWeakArray<ShaderBinaryMutation>(&dummyMutation, 1);
				}

				for(const ShaderBinaryMutation& mutation : mutations)
				{
					const ShaderBinaryVariant& variant = binary->m_variants[mutation.m_variantIndex];
					const U32 codeBlockIndex = variant.m_techniqueCodeBlocks[techniqueIdx].m_codeBlockIndices[ShaderType::kRayGen];
					const U32 shaderIdx = lib->addShader(binary->m_codeBlocks[codeBlockIndex], progName, ShaderType::kRayGen);

					lib->addGroup(filename, mutation.m_hash, shaderIdx, kMaxU32, kMaxU32, kMaxU32);
				}
			}

			// Miss
			if(!!(technique.m_shaderTypes & ShaderTypeBit::kMiss))
			{
				// Iterate all mutations
				ConstWeakArray<ShaderBinaryMutation> mutations;
				ShaderBinaryMutation dummyMutation;
				if(binary->m_mutations.getSize() > 1)
				{
					mutations = binary->m_mutations;
				}
				else
				{
					dummyMutation.m_hash = 0;
					dummyMutation.m_variantIndex = 0;
					mutations = ConstWeakArray<ShaderBinaryMutation>(&dummyMutation, 1);
				}

				for(const ShaderBinaryMutation& mutation : mutations)
				{
					const ShaderBinaryVariant& variant = binary->m_variants[mutation.m_variantIndex];
					const U32 codeBlockIndex = variant.m_techniqueCodeBlocks[techniqueIdx].m_codeBlockIndices[ShaderType::kMiss];
					ANKI_ASSERT(codeBlockIndex < kMaxU32);
					const U32 shaderIdx = lib->addShader(binary->m_codeBlocks[codeBlockIndex], progName, ShaderType::kMiss);

					lib->addGroup(filename, mutation.m_hash, kMaxU32, shaderIdx, kMaxU32, kMaxU32);
				}
			}

			// Hit shaders
			if(!!(technique.m_shaderTypes & ShaderTypeBit::kAllHit))
			{
				// Before you iterate the mutations do some work if there are none
				ConstWeakArray<ShaderBinaryMutation> mutations;
				ShaderBinaryMutation dummyMutation;
				if(binary->m_mutations.getSize() > 1)
				{
					mutations = binary->m_mutations;
				}
				else
				{
					dummyMutation.m_hash = 0;
					dummyMutation.m_variantIndex = 0;
					mutations = ConstWeakArray<ShaderBinaryMutation>(&dummyMutation, 1);
				}

				// Iterate all mutations
				for(const ShaderBinaryMutation& mutation : mutations)
				{
					if(mutation.m_variantIndex == kMaxU32)
					{
						continue;
					}

					const ShaderBinaryVariant& variant = binary->m_variants[mutation.m_variantIndex];
					const U32 ahitCodeBlockIndex = variant.m_techniqueCodeBlocks[techniqueIdx].m_codeBlockIndices[ShaderType::kAnyHit];
					const U32 chitCodeBlockIndex = variant.m_techniqueCodeBlocks[techniqueIdx].m_codeBlockIndices[ShaderType::kClosestHit];
					ANKI_ASSERT(ahitCodeBlockIndex != kMaxU32 || chitCodeBlockIndex != kMaxU32);

					const U32 ahitShaderIdx = (ahitCodeBlockIndex != kMaxU32)
												  ? lib->addShader(binary->m_codeBlocks[ahitCodeBlockIndex], progName, ShaderType::kAnyHit)
												  : kMaxU32;

					const U32 chitShaderIdx = (chitCodeBlockIndex != kMaxU32)
												  ? lib->addShader(binary->m_codeBlocks[chitCodeBlockIndex], progName, ShaderType::kClosestHit)
												  : kMaxU32;

					lib->addGroup(filename, mutation.m_hash, kMaxU32, kMaxU32, chitShaderIdx, ahitShaderIdx);
				}
			}
		} // iterate techniques

		return FunctorContinue::kContinue;
	}); // For all RT filenames
	ANKI_CHECK(err);

	// Create the libraries. The value that goes to the m_resourceHashToShaderGroupHandleIndex hashmap is the index of the shader handle inside the
	// program. Leverage the fact that there is a predefined order between group types (ray gen first, miss and hit follows). See the ShaderProgram
	// class for info.
	if(libs.getSize() != 0)
	{
		outLibs.resize(libs.getSize());

		for(U32 libIdx = 0; libIdx < libs.getSize(); ++libIdx)
		{
			ShaderProgramRaytracingLibrary& outLib = outLibs[libIdx];
			const Lib& inLib = libs[libIdx];

			const ShaderTypeBit requiredShaders = ShaderTypeBit::kRayGen | ShaderTypeBit::kMiss;
			if((inLib.m_presentStages & requiredShaders) != requiredShaders
			   || !(inLib.m_presentStages & (ShaderTypeBit::kClosestHit | ShaderTypeBit::kAnyHit)))
			{
				ANKI_RESOURCE_LOGE("The libray is missing shader types: %s", inLib.m_name.cstr());
				return Error::kUserData;
			}

			outLib.m_libraryName = inLib.m_name;

			ResourceDynamicArray<RayTracingHitGroup> initInfoHitGroups;
			ResourceDynamicArray<Shader*> missShaders;
			ResourceDynamicArray<Shader*> rayGenShaders;
			U32 groupCount = 0;

			// Ray gen
			for(const ShaderGroup& group : inLib.m_rayGenGroups)
			{
				rayGenShaders.emplaceBack(inLib.m_shaders[group.m_rayGen].m_shader.get());

				for(U64 hash : group.m_groupHashes)
				{
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(hash, groupCount);
				}

				++groupCount;
			}

			// Miss
			for(const ShaderGroup& group : inLib.m_missGroups)
			{
				missShaders.emplaceBack(inLib.m_shaders[group.m_miss].m_shader.get());

				for(U64 hash : group.m_groupHashes)
				{
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(hash, groupCount);
				}

				++groupCount;
			}

			// Hit
			for(const ShaderGroup& group : inLib.m_hitGroups)
			{
				RayTracingHitGroup* infoHitGroup = initInfoHitGroups.emplaceBack();

				if(group.m_ahit < kMaxU32)
				{
					infoHitGroup->m_anyHitShader = inLib.m_shaders[group.m_ahit].m_shader.get();
				}

				if(group.m_chit < kMaxU32)
				{
					infoHitGroup->m_closestHitShader = inLib.m_shaders[group.m_chit].m_shader.get();
				}

				for(U64 hash : group.m_groupHashes)
				{
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(hash, groupCount);
				}

				++groupCount;
			}

			// Create the program
			ShaderProgramInitInfo inf(inLib.m_name);
			inf.m_rayTracingShaders.m_rayGenShaders = rayGenShaders;
			inf.m_rayTracingShaders.m_missShaders = missShaders;
			inf.m_rayTracingShaders.m_hitGroups = initInfoHitGroups;
			outLib.m_program = GrManager::getSingleton().newShaderProgram(inf);

			// Upload the group handles
			{
				ConstWeakArray<U8> handlesMem = outLib.m_program->getShaderGroupHandles();
				outLib.m_shaderGroupHandlesBuff =
					TextureMemoryPool::getSingleton().allocateStructuredBuffer<U32>(U32(handlesMem.getSizeInBytes() / sizeof(U32)));
				WeakArray<U8> tmpMem;
				BufferView tmpBuff = RebarTransientMemoryPool::getSingleton().allocateCopyBuffer(U32(handlesMem.getSizeInBytes()), tmpMem);
				memcpy(tmpMem.getBegin(), handlesMem.getBegin(), handlesMem.getSizeInBytes());

				CommandBufferInitInfo buffInit("RT handles copy");
				buffInit.m_flags = CommandBufferFlag::kGeneralWork | CommandBufferFlag::kSmallBatch;
				CommandBufferPtr cmdb = GrManager::getSingleton().newCommandBuffer(buffInit);
				cmdb->copyBufferToBuffer(tmpBuff, outLib.m_shaderGroupHandlesBuff);
				cmdb->endRecording();

				FencePtr fence;
				GrManager::getSingleton().submit(cmdb.get(), {}, &fence);
				if(!fence->clientWait(kMaxSecond))
				{
					ANKI_RESOURCE_LOGE("GPU timeout detected");
					return Error::kFunctionFailed;
				}
			}

			++rtProgramCount;
		}
	}

	ANKI_RESOURCE_LOGI("Created %u ray tracing programs", rtProgramCount);

	return Error::kNone;
}

} // end namespace anki
