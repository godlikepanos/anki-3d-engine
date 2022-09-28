// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Resource/ShaderProgramResourceSystem.h>
#include <AnKi/Resource/ResourceFilesystem.h>
#include <AnKi/Util/Tracer.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/ShaderCompiler/ShaderProgramCompiler.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/ThreadHive.h>
#include <AnKi/Util/System.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Core/ConfigSet.h>

namespace anki {

U64 ShaderProgramRaytracingLibrary::generateShaderGroupGroupHash(CString resourceFilename, U64 mutationHash,
																 HeapMemoryPool& pool)
{
	ANKI_ASSERT(resourceFilename.getLength() > 0);
	StringRaii basename(&pool);
	getFilepathFilename(resourceFilename, basename);
	const U64 hash = appendHash(basename.cstr(), basename.getLength(), mutationHash);
	return hash;
}

ShaderProgramResourceSystem::~ShaderProgramResourceSystem()
{
	m_rtLibraries.destroy(*m_pool);
}

Error ShaderProgramResourceSystem::init(ResourceFilesystem& fs, GrManager& gr)
{
	if(!gr.getDeviceCapabilities().m_rayTracingEnabled)
	{
		return Error::kNone;
	}

	// Create RT pipeline libraries
	const Error err = createRayTracingPrograms(fs, gr, *m_pool, m_rtLibraries);
	if(err)
	{
		ANKI_RESOURCE_LOGE("Failed to create ray tracing programs");
	}

	return err;
}

Error ShaderProgramResourceSystem::createRayTracingPrograms(ResourceFilesystem& fs, GrManager& gr, HeapMemoryPool& pool,
															DynamicArray<ShaderProgramRaytracingLibrary>& outLibs)
{
	ANKI_RESOURCE_LOGI("Creating ray tracing programs");
	U32 rtProgramCount = 0;

	class Shader
	{
	public:
		ShaderPtr m_shader;
		U64 m_hash = 0; ///< Hash of the binary.
	};

	class ShaderGroup
	{
	public:
		U32 m_rayGen = kMaxU32;
		U32 m_miss = kMaxU32;
		U32 m_chit = kMaxU32;
		U32 m_ahit = kMaxU32;
		U64 m_hitGroupHash = 0;
	};

	class Lib
	{
	public:
		HeapMemoryPool* m_pool = nullptr;
		GrManager* m_gr;
		StringRaii m_name = {m_pool};
		DynamicArrayRaii<Shader> m_shaders = {m_pool};
		DynamicArrayRaii<ShaderGroup> m_shaderGroups = {m_pool};
		ShaderTypeBit m_presentStages = ShaderTypeBit::kNone;
		U32 m_rayTypeCount = 0;
		BitSet<64> m_rayTypeMask = {false};
		U32 m_rayGenShaderGroupCount = 0;
		U32 m_missShaderGroupCount = 0;
		U32 m_hitShaderGroupCount = 0;

		Lib(HeapMemoryPool* pool, GrManager* gr)
			: m_pool(pool)
			, m_gr(gr)
		{
			ANKI_ASSERT(pool);
		}

		U32 addShader(const ShaderProgramBinaryCodeBlock& codeBlock, CString progName, ShaderType shaderType)
		{
			Shader* shader = nullptr;
			for(Shader& s : m_shaders)
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
				shader->m_shader = m_gr->newShader(inf);
				shader->m_hash = codeBlock.m_hash;

				m_presentStages |= ShaderTypeBit(1 << shaderType);
			}

			return U32(shader - m_shaders.getBegin());
		}

		void addGroup(CString filename, U64 mutationHash, U32 rayGen, U32 miss, U32 chit, U32 ahit)
		{
			const U64 groupHash =
				ShaderProgramRaytracingLibrary::generateShaderGroupGroupHash(filename, mutationHash, *m_pool);
#if ANKI_ENABLE_ASSERTIONS
			for(const ShaderGroup& group : m_shaderGroups)
			{
				ANKI_ASSERT(group.m_hitGroupHash != groupHash && "Shouldn't find group with the same hash");
			}
#endif

			ShaderGroup group;
			group.m_rayGen = rayGen;
			group.m_miss = miss;
			group.m_chit = chit;
			group.m_ahit = ahit;
			group.m_hitGroupHash = groupHash;
			m_shaderGroups.emplaceBack(group);

			if(rayGen < kMaxU32)
			{
				++m_rayGenShaderGroupCount;
			}
			else if(miss < kMaxU32)
			{
				++m_missShaderGroupCount;
			}
			else
			{
				ANKI_ASSERT(chit < kMaxU32 || ahit < kMaxU32);
				++m_hitShaderGroupCount;
			}
		}
	};

	DynamicArrayRaii<Lib> libs(&pool);

	ANKI_CHECK(fs.iterateAllFilenames([&](CString filename) -> Error {
		// Check file extension
		StringRaii extension(&pool);
		getFilepathExtension(filename, extension);
		const Char binExtension[] = "ankiprogbin";
		if(extension.getLength() != sizeof(binExtension) - 1 || extension != binExtension)
		{
			return Error::kNone;
		}

		if(filename.find("ShaderBinaries/Rt") != 0)
		{
			// Doesn't start with the expected path, skip it
			return Error::kNone;
		}

		// Get the binary
		ResourceFilePtr file;
		ANKI_CHECK(fs.openFile(filename, file));
		ShaderProgramBinaryWrapper binaryw(&pool);
		ANKI_CHECK(binaryw.deserializeFromAnyFile(*file));
		const ShaderProgramBinary& binary = binaryw.getBinary();

		if(!(binary.m_presentShaderTypes & ShaderTypeBit::kAllRayTracing))
		{
			return Error::kNone;
		}

		// Checks
		if(binary.m_libraryName[0] == '\0')
		{
			ANKI_RESOURCE_LOGE("Library is missing from program: %s", filename.cstr());
			return Error::kUserData;
		}

		// Create the program name
		StringRaii progName(&pool);
		getFilepathFilename(filename, progName);

		// Find or create the lib
		Lib* lib = nullptr;
		{
			for(Lib& l : libs)
			{
				if(l.m_name == CString(&binary.m_libraryName[0]))
				{
					lib = &l;
					break;
				}
			}

			if(lib == nullptr)
			{
				libs.emplaceBack(&pool, &gr);
				lib = &libs.getBack();
				lib->m_name.create(CString(&binary.m_libraryName[0]));
			}
		}

		// Update the ray type
		const U32 rayTypeNumber = binary.m_rayType;
		if(rayTypeNumber != kMaxU32)
		{
			lib->m_rayTypeCount = max(lib->m_rayTypeCount, rayTypeNumber + 1);
			lib->m_rayTypeMask.set(rayTypeNumber);
		}

		// Ray gen
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::kRayGen))
		{
			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::kRayGen))
			{
				ANKI_RESOURCE_LOGE("Ray gen can't co-exist with other types: %s", filename.cstr());
				return Error::kUserData;
			}

			if(binary.m_constants.getSize())
			{
				ANKI_RESOURCE_LOGE("Ray gen can't have spec constants ATM: %s", filename.cstr());
				return Error::kUserData;
			}

			// Iterate all mutations
			ConstWeakArray<ShaderProgramBinaryMutation> mutations;
			ShaderProgramBinaryMutation dummyMutation;
			if(binary.m_mutations.getSize() > 1)
			{
				mutations = binary.m_mutations;
			}
			else
			{
				dummyMutation.m_hash = 0;
				dummyMutation.m_variantIndex = 0;
				mutations = ConstWeakArray<ShaderProgramBinaryMutation>(&dummyMutation, 1);
			}

			for(const ShaderProgramBinaryMutation& mutation : mutations)
			{
				const ShaderProgramBinaryVariant& variant = binary.m_variants[mutation.m_variantIndex];
				const U32 codeBlockIndex = variant.m_codeBlockIndices[ShaderType::kRayGen];
				ANKI_ASSERT(codeBlockIndex != kMaxU32);
				const U32 shaderIdx =
					lib->addShader(binary.m_codeBlocks[codeBlockIndex], progName, ShaderType::kRayGen);

				lib->addGroup(filename, mutation.m_hash, shaderIdx, kMaxU32, kMaxU32, kMaxU32);
			}
		}

		// Miss shaders
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::kMiss))
		{
			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::kMiss))
			{
				ANKI_RESOURCE_LOGE("Miss shaders can't co-exist with other types: %s", filename.cstr());
				return Error::kUserData;
			}

			if(binary.m_constants.getSize())
			{
				ANKI_RESOURCE_LOGE("Miss can't have spec constants ATM: %s", filename.cstr());
				return Error::kUserData;
			}

			if(rayTypeNumber == kMaxU32)
			{
				ANKI_RESOURCE_LOGE("Miss shader should have set the ray type: %s", filename.cstr());
				return Error::kUserData;
			}

			// Iterate all mutations
			ConstWeakArray<ShaderProgramBinaryMutation> mutations;
			ShaderProgramBinaryMutation dummyMutation;
			if(binary.m_mutations.getSize() > 1)
			{
				mutations = binary.m_mutations;
			}
			else
			{
				dummyMutation.m_hash = 0;
				dummyMutation.m_variantIndex = 0;
				mutations = ConstWeakArray<ShaderProgramBinaryMutation>(&dummyMutation, 1);
			}

			for(const ShaderProgramBinaryMutation& mutation : mutations)
			{
				const ShaderProgramBinaryVariant& variant = binary.m_variants[mutation.m_variantIndex];
				const U32 codeBlockIndex = variant.m_codeBlockIndices[ShaderType::kMiss];
				ANKI_ASSERT(codeBlockIndex != kMaxU32);
				const U32 shaderIdx = lib->addShader(binary.m_codeBlocks[codeBlockIndex], progName, ShaderType::kMiss);

				lib->addGroup(filename, mutation.m_hash, kMaxU32, shaderIdx, kMaxU32, kMaxU32);
			}
		}

		// Hit shaders
		if(!!(binary.m_presentShaderTypes & (ShaderTypeBit::kAnyHit | ShaderTypeBit::kClosestHit)))
		{
			if(!!(binary.m_presentShaderTypes & ~(ShaderTypeBit::kAnyHit | ShaderTypeBit::kClosestHit)))
			{
				ANKI_RESOURCE_LOGE("Hit shaders can't co-exist with other types: %s", filename.cstr());
				return Error::kUserData;
			}

			if(rayTypeNumber == kMaxU32)
			{
				ANKI_RESOURCE_LOGE("Hit shaders should have set the ray type: %s", filename.cstr());
				return Error::kUserData;
			}

			// Before you iterate the mutations do some work if there are none
			ConstWeakArray<ShaderProgramBinaryMutation> mutations;
			ShaderProgramBinaryMutation dummyMutation;
			if(binary.m_mutations.getSize() > 1)
			{
				mutations = binary.m_mutations;
			}
			else
			{
				dummyMutation.m_hash = 0;
				dummyMutation.m_variantIndex = 0;
				mutations = ConstWeakArray<ShaderProgramBinaryMutation>(&dummyMutation, 1);
			}

			// Iterate all mutations
			for(const ShaderProgramBinaryMutation& mutation : mutations)
			{
				const ShaderProgramBinaryVariant& variant = binary.m_variants[mutation.m_variantIndex];
				const U32 ahitCodeBlockIndex = variant.m_codeBlockIndices[ShaderType::kAnyHit];
				const U32 chitCodeBlockIndex = variant.m_codeBlockIndices[ShaderType::kClosestHit];
				ANKI_ASSERT(ahitCodeBlockIndex != kMaxU32 || chitCodeBlockIndex != kMaxU32);

				const U32 ahitShaderIdx =
					(ahitCodeBlockIndex != kMaxU32)
						? lib->addShader(binary.m_codeBlocks[ahitCodeBlockIndex], progName, ShaderType::kAnyHit)
						: kMaxU32;

				const U32 chitShaderIdx =
					(chitCodeBlockIndex != kMaxU32)
						? lib->addShader(binary.m_codeBlocks[chitCodeBlockIndex], progName, ShaderType::kClosestHit)
						: kMaxU32;

				lib->addGroup(filename, mutation.m_hash, kMaxU32, kMaxU32, chitShaderIdx, ahitShaderIdx);
			}
		}

		return Error::kNone;
	})); // For all RT filenames

	// Create the libraries the value that goes to the m_resourceHashToShaderGroupHandleIndex hashmap is the index of
	// the shader handle inside the program. Leverage the fact that there is a predefined order between shader types.
	// See the ShaderProgram class for info.
	if(libs.getSize() != 0)
	{
		outLibs.create(pool, libs.getSize());

		for(U32 libIdx = 0; libIdx < libs.getSize(); ++libIdx)
		{
			ShaderProgramRaytracingLibrary& outLib = outLibs[libIdx];
			const Lib& inLib = libs[libIdx];

			outLib.m_pool = &pool;

			if(inLib.m_presentStages
			   != (ShaderTypeBit::kRayGen | ShaderTypeBit::kMiss | ShaderTypeBit::kClosestHit | ShaderTypeBit::kAnyHit))
			{
				ANKI_RESOURCE_LOGE("The libray is missing shader shader types: %s", inLib.m_name.cstr());
				return Error::kUserData;
			}

			if(inLib.m_rayTypeCount != inLib.m_rayTypeMask.getEnabledBitCount())
			{
				ANKI_RESOURCE_LOGE("Ray types are not contiguous for library: %s", inLib.m_name.cstr());
				return Error::kUserData;
			}

			outLib.m_libraryName.create(pool, inLib.m_name);
			outLib.m_rayTypeCount = inLib.m_rayTypeCount;

			DynamicArrayRaii<RayTracingHitGroup> initInfoHitGroups(&pool);
			DynamicArrayRaii<ShaderPtr> missShaders(&pool);
			DynamicArrayRaii<ShaderPtr> rayGenShaders(&pool);

			// Add the hitgroups to the init info
			for(U32 shaderGroupIdx = 0; shaderGroupIdx < inLib.m_shaderGroups.getSize(); ++shaderGroupIdx)
			{
				const ShaderGroup& inShaderGroup = inLib.m_shaderGroups[shaderGroupIdx];
				ANKI_ASSERT(inShaderGroup.m_hitGroupHash != 0);

				if(inShaderGroup.m_ahit < kMaxU32 || inShaderGroup.m_chit < kMaxU32)
				{
					// Hit shaders

					ANKI_ASSERT(inShaderGroup.m_miss == kMaxU32 && inShaderGroup.m_rayGen == kMaxU32);
					RayTracingHitGroup* infoHitGroup = initInfoHitGroups.emplaceBack();

					if(inShaderGroup.m_ahit < kMaxU32)
					{
						infoHitGroup->m_anyHitShader = inLib.m_shaders[inShaderGroup.m_ahit].m_shader;
					}

					if(inShaderGroup.m_chit < kMaxU32)
					{
						infoHitGroup->m_closestHitShader = inLib.m_shaders[inShaderGroup.m_chit].m_shader;
					}

					// The hit shaders are after ray gen and miss shaders
					const U32 idx =
						inLib.m_rayGenShaderGroupCount + inLib.m_missShaderGroupCount + initInfoHitGroups.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(pool, inShaderGroup.m_hitGroupHash, idx);
				}
				else if(inShaderGroup.m_miss < kMaxU32)
				{
					// Miss shader

					ANKI_ASSERT(inShaderGroup.m_ahit == kMaxU32 && inShaderGroup.m_chit == kMaxU32
								&& inShaderGroup.m_rayGen == kMaxU32);

					missShaders.emplaceBack(inLib.m_shaders[inShaderGroup.m_miss].m_shader);

					// The miss shaders are after ray gen
					const U32 idx = inLib.m_rayGenShaderGroupCount + missShaders.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(pool, inShaderGroup.m_hitGroupHash, idx);
				}
				else
				{
					// Ray gen shader

					ANKI_ASSERT(inShaderGroup.m_ahit == kMaxU32 && inShaderGroup.m_chit == kMaxU32
								&& inShaderGroup.m_miss == kMaxU32 && inShaderGroup.m_rayGen < kMaxU32);

					rayGenShaders.emplaceBack(inLib.m_shaders[inShaderGroup.m_rayGen].m_shader);

					// Ray gen shaders are first
					const U32 idx = rayGenShaders.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(pool, inShaderGroup.m_hitGroupHash, idx);
				}
			} // end for all groups

			// Create the program
			ShaderProgramInitInfo inf(inLib.m_name);
			inf.m_rayTracingShaders.m_rayGenShaders = rayGenShaders;
			inf.m_rayTracingShaders.m_missShaders = missShaders;
			inf.m_rayTracingShaders.m_hitGroups = initInfoHitGroups;
			outLib.m_program = gr.newShaderProgram(inf);

			++rtProgramCount;
		}
	}

	ANKI_RESOURCE_LOGI("Created %u ray tracing programs", rtProgramCount);

	return Error::kNone;
}

} // end namespace anki
