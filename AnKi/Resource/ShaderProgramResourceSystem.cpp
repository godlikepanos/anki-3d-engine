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
																 GenericMemoryPoolAllocator<U8> alloc)
{
	ANKI_ASSERT(resourceFilename.getLength() > 0);
	StringAuto basename(alloc);
	getFilepathFilename(resourceFilename, basename);
	const U64 hash = appendHash(basename.cstr(), basename.getLength(), mutationHash);
	return hash;
}

ShaderProgramResourceSystem::~ShaderProgramResourceSystem()
{
	m_cacheDir.destroy(m_alloc);
	m_rtLibraries.destroy(m_alloc);
}

Error ShaderProgramResourceSystem::init()
{
	ANKI_TRACE_SCOPED_EVENT(COMPILE_SHADERS);

	StringListAuto rtProgramFilenames(m_alloc);
	ANKI_CHECK(compileAllShaders(m_cacheDir, *m_gr, *m_fs, m_alloc, rtProgramFilenames));

	if(m_gr->getDeviceCapabilities().m_rayTracingEnabled)
	{
		ANKI_CHECK(createRayTracingPrograms(m_cacheDir, rtProgramFilenames, *m_gr, m_alloc, m_rtLibraries));
	}

	return Error::NONE;
}

Error ShaderProgramResourceSystem::compileAllShaders(CString cacheDir, GrManager& gr, ResourceFilesystem& fs,
													 GenericMemoryPoolAllocator<U8>& alloc,
													 StringListAuto& rtProgramFilenames)
{
	class MetaFileData
	{
	public:
		U64 m_hash;
		ShaderTypeBit m_shaderTypes;
		Array<U16, 3> m_padding = {};
	};

	ANKI_RESOURCE_LOGI("Compiling shader programs");
	U32 shadersCompileCount = 0;
	U32 shadersTotalCount = 0;

	ThreadHive threadHive(getCpuCoresCount(), alloc, false);

	// Compute hash for both
	ShaderCompilerOptions compilerOptions;
	compilerOptions.m_forceFullFloatingPointPrecision = gr.getConfig().getRsrcForceFullFpPrecision();
	compilerOptions.m_mobilePlatform = ANKI_PLATFORM_MOBILE;
	U64 gpuHash = computeHash(&compilerOptions, sizeof(compilerOptions));
	gpuHash = appendHash(&SHADER_BINARY_VERSION, sizeof(SHADER_BINARY_VERSION), gpuHash);

	ANKI_CHECK(fs.iterateAllFilenames([&](CString fname) -> Error {
		// Check file extension
		StringAuto extension(alloc);
		getFilepathExtension(fname, extension);
		if(extension.getLength() != 8 || extension != "ankiprog")
		{
			return Error::NONE;
		}

		++shadersTotalCount;

		if(fname.find("/Rt") != CString::NPOS && !gr.getDeviceCapabilities().m_rayTracingEnabled)
		{
			// Skip RT programs when RT is disabled
			return Error::NONE;
		}

		// Get some filenames
		StringAuto baseFname(alloc);
		getFilepathFilename(fname, baseFname);
		StringAuto metaFname(alloc);
		metaFname.sprintf("%s/%smeta", cacheDir.cstr(), baseFname.cstr());

		// Get the hash from the meta file
		U64 metafileHash = 0;
		ShaderTypeBit metafileShaderTypes = ShaderTypeBit::NONE;
		if(fileExists(metaFname))
		{
			File metaFile;
			ANKI_CHECK(metaFile.open(metaFname, FileOpenFlag::READ | FileOpenFlag::BINARY));
			MetaFileData data;
			ANKI_CHECK(metaFile.read(&data, sizeof(data)));

			if(data.m_hash == 0 || data.m_shaderTypes == ShaderTypeBit::NONE)
			{
				ANKI_RESOURCE_LOGE("Wrong data found in the metafile: %s", metaFname.cstr());
				return Error::USER_DATA;
			}

			metafileHash = data.m_hash;
			metafileShaderTypes = data.m_shaderTypes;
		}

		// Load interface
		class FSystem : public ShaderProgramFilesystemInterface
		{
		public:
			ResourceFilesystem* m_fsystem = nullptr;

			Error readAllText(CString filename, StringAuto& txt) final
			{
				ResourceFilePtr file;
				ANKI_CHECK(m_fsystem->openFile(filename, file));
				ANKI_CHECK(file->readAllText(txt));
				return Error::NONE;
			}
		} fsystem;
		fsystem.m_fsystem = &fs;

		// Skip interface
		class Skip : public ShaderProgramPostParseInterface
		{
		public:
			U64 m_metafileHash;
			U64 m_newHash;
			U64 m_gpuHash;
			CString m_fname;

			Bool skipCompilation(U64 hash)
			{
				ANKI_ASSERT(hash != 0);
				const Array<U64, 2> hashes = {hash, m_gpuHash};
				const U64 finalHash = computeHash(hashes.getBegin(), hashes.getSizeInBytes());

				m_newHash = finalHash;
				const Bool skip = finalHash == m_metafileHash;

				if(!skip)
				{
					ANKI_RESOURCE_LOGI("\t%s", m_fname.cstr());
				}

				return skip;
			};
		} skip;
		skip.m_metafileHash = metafileHash;
		skip.m_newHash = 0;
		skip.m_gpuHash = gpuHash;
		skip.m_fname = fname;

		// Threading interface
		class TaskManager : public ShaderProgramAsyncTaskInterface
		{
		public:
			ThreadHive* m_hive = nullptr;
			GenericMemoryPoolAllocator<U8> m_alloc;

			void enqueueTask(void (*callback)(void* userData), void* userData)
			{
				class Ctx
				{
				public:
					void (*m_callback)(void* userData);
					void* m_userData;
					GenericMemoryPoolAllocator<U8> m_alloc;
				};
				Ctx* ctx = m_alloc.newInstance<Ctx>();
				ctx->m_callback = callback;
				ctx->m_userData = userData;
				ctx->m_alloc = m_alloc;

				m_hive->submitTask(
					[](void* userData, U32 threadId, ThreadHive& hive, ThreadHiveSemaphore* signalSemaphore) {
						Ctx* ctx = static_cast<Ctx*>(userData);
						ctx->m_callback(ctx->m_userData);
						auto alloc = ctx->m_alloc;
						alloc.deleteInstance(ctx);
					},
					ctx);
			}

			Error joinTasks()
			{
				m_hive->waitAllTasks();
				return Error::NONE;
			}
		} taskManager;
		taskManager.m_hive = &threadHive;
		taskManager.m_alloc = alloc;

		// Compile
		ShaderProgramBinaryWrapper binary(alloc);
		ANKI_CHECK(compileShaderProgram(fname, fsystem, &skip, &taskManager, alloc, compilerOptions, binary));

		const Bool cachedBinIsUpToDate = metafileHash == skip.m_newHash;
		if(!cachedBinIsUpToDate)
		{
			++shadersCompileCount;
		}

		// Update the meta file
		if(!cachedBinIsUpToDate)
		{
			File metaFile;
			ANKI_CHECK(metaFile.open(metaFname, FileOpenFlag::WRITE | FileOpenFlag::BINARY));

			MetaFileData data;
			data.m_hash = skip.m_newHash;
			data.m_shaderTypes = binary.getBinary().m_presentShaderTypes;
			metafileShaderTypes = data.m_shaderTypes;
			ANKI_CHECK(metaFile.write(&data, sizeof(data)));
		}

		// Save the binary to the cache
		if(!cachedBinIsUpToDate)
		{
			StringAuto storeFname(alloc);
			storeFname.sprintf("%s/%sbin", cacheDir.cstr(), baseFname.cstr());
			ANKI_CHECK(binary.serializeToFile(storeFname));
		}

		// Gather RT programs
		if(!!(metafileShaderTypes & ShaderTypeBit::ALL_RAY_TRACING))
		{
			rtProgramFilenames.pushBack(fname);
		}

		return Error::NONE;
	}));

	ANKI_RESOURCE_LOGI("Compiled %u shader programs out of %u", shadersCompileCount, shadersTotalCount);
	return Error::NONE;
}

Error ShaderProgramResourceSystem::createRayTracingPrograms(CString cacheDir, const StringListAuto& rtProgramFilenames,
															GrManager& gr, GenericMemoryPoolAllocator<U8>& alloc,
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
		U32 m_rayGen = MAX_U32;
		U32 m_miss = MAX_U32;
		U32 m_chit = MAX_U32;
		U32 m_ahit = MAX_U32;
		U64 m_hitGroupHash = 0;
	};

	class Lib
	{
	public:
		GenericMemoryPoolAllocator<U8> m_alloc;
		GrManager* m_gr;
		StringAuto m_name{m_alloc};
		DynamicArrayAuto<Shader> m_shaders{m_alloc};
		DynamicArrayAuto<ShaderGroup> m_shaderGroups{m_alloc};
		ShaderTypeBit m_presentStages = ShaderTypeBit::NONE;
		U32 m_rayTypeCount = 0;
		BitSet<64> m_rayTypeMask = {false};
		U32 m_rayGenShaderGroupCount = 0;
		U32 m_missShaderGroupCount = 0;
		U32 m_hitShaderGroupCount = 0;

		Lib(GenericMemoryPoolAllocator<U8> alloc, GrManager* gr)
			: m_alloc(alloc)
			, m_gr(gr)
		{
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
				ShaderProgramRaytracingLibrary::generateShaderGroupGroupHash(filename, mutationHash, m_alloc);
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

			if(rayGen < MAX_U32)
			{
				++m_rayGenShaderGroupCount;
			}
			else if(miss < MAX_U32)
			{
				++m_missShaderGroupCount;
			}
			else
			{
				ANKI_ASSERT(chit < MAX_U32 || ahit < MAX_U32);
				++m_hitShaderGroupCount;
			}
		}
	};

	DynamicArrayAuto<Lib> libs(alloc);

	for(const String& filename : rtProgramFilenames)
	{
		// Get the binary
		StringAuto baseFilename(alloc);
		getFilepathFilename(filename, baseFilename);
		StringAuto binaryFilename(alloc);
		binaryFilename.sprintf("%s/%sbin", cacheDir.cstr(), baseFilename.cstr());
		ShaderProgramBinaryWrapper binaryw(alloc);
		ANKI_CHECK(binaryw.deserializeFromFile(binaryFilename));
		const ShaderProgramBinary& binary = binaryw.getBinary();

		// Checks
		if(binary.m_libraryName[0] == '\0')
		{
			ANKI_RESOURCE_LOGE("Library is missing from program: %s", filename.cstr());
			return Error::USER_DATA;
		}

		// Create the program name
		StringAuto progName(alloc);
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
				libs.emplaceBack(alloc, &gr);
				lib = &libs.getBack();
				lib->m_name.create(CString(&binary.m_libraryName[0]));
			}
		}

		// Update the ray type
		const U32 rayTypeNumber = binary.m_rayType;
		if(rayTypeNumber != MAX_U32)
		{
			lib->m_rayTypeCount = max(lib->m_rayTypeCount, rayTypeNumber + 1);
			lib->m_rayTypeMask.set(rayTypeNumber);
		}

		// Ray gen
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::RAY_GEN))
		{
			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::RAY_GEN))
			{
				ANKI_RESOURCE_LOGE("Ray gen can't co-exist with other types: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(binary.m_constants.getSize())
			{
				ANKI_RESOURCE_LOGE("Ray gen can't have spec constants ATM: %s", filename.cstr());
				return Error::USER_DATA;
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
				const U32 codeBlockIndex = variant.m_codeBlockIndices[ShaderType::RAY_GEN];
				ANKI_ASSERT(codeBlockIndex != MAX_U32);
				const U32 shaderIdx =
					lib->addShader(binary.m_codeBlocks[codeBlockIndex], progName, ShaderType::RAY_GEN);

				lib->addGroup(filename, mutation.m_hash, shaderIdx, MAX_U32, MAX_U32, MAX_U32);
			}
		}

		// Miss shaders
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::MISS))
		{
			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::MISS))
			{
				ANKI_RESOURCE_LOGE("Miss shaders can't co-exist with other types: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(binary.m_constants.getSize())
			{
				ANKI_RESOURCE_LOGE("Miss can't have spec constants ATM: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(rayTypeNumber == MAX_U32)
			{
				ANKI_RESOURCE_LOGE("Miss shader should have set the ray type: %s", filename.cstr());
				return Error::USER_DATA;
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
				const U32 codeBlockIndex = variant.m_codeBlockIndices[ShaderType::MISS];
				ANKI_ASSERT(codeBlockIndex != MAX_U32);
				const U32 shaderIdx = lib->addShader(binary.m_codeBlocks[codeBlockIndex], progName, ShaderType::MISS);

				lib->addGroup(filename, mutation.m_hash, MAX_U32, shaderIdx, MAX_U32, MAX_U32);
			}
		}

		// Hit shaders
		if(!!(binary.m_presentShaderTypes & (ShaderTypeBit::ANY_HIT | ShaderTypeBit::CLOSEST_HIT)))
		{
			if(!!(binary.m_presentShaderTypes & ~(ShaderTypeBit::ANY_HIT | ShaderTypeBit::CLOSEST_HIT)))
			{
				ANKI_RESOURCE_LOGE("Hit shaders can't co-exist with other types: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(rayTypeNumber == MAX_U32)
			{
				ANKI_RESOURCE_LOGE("Hit shaders should have set the ray type: %s", filename.cstr());
				return Error::USER_DATA;
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
				const U32 ahitCodeBlockIndex = variant.m_codeBlockIndices[ShaderType::ANY_HIT];
				const U32 chitCodeBlockIndex = variant.m_codeBlockIndices[ShaderType::CLOSEST_HIT];
				ANKI_ASSERT(ahitCodeBlockIndex != MAX_U32 || chitCodeBlockIndex != MAX_U32);

				const U32 ahitShaderIdx =
					(ahitCodeBlockIndex != MAX_U32)
						? lib->addShader(binary.m_codeBlocks[ahitCodeBlockIndex], progName, ShaderType::ANY_HIT)
						: MAX_U32;

				const U32 chitShaderIdx =
					(chitCodeBlockIndex != MAX_U32)
						? lib->addShader(binary.m_codeBlocks[chitCodeBlockIndex], progName, ShaderType::CLOSEST_HIT)
						: MAX_U32;

				lib->addGroup(filename, mutation.m_hash, MAX_U32, MAX_U32, chitShaderIdx, ahitShaderIdx);
			}
		}
	} // For all RT filenames

	// Create the libraries the value that goes to the m_resourceHashToShaderGroupHandleIndex hashmap is the index of
	// the shader handle inside the program. Leverage the fact that there is a predefined order between shader types.
	// See the ShaderProgram class for info.
	if(libs.getSize() != 0)
	{
		outLibs.create(alloc, libs.getSize());

		for(U32 libIdx = 0; libIdx < libs.getSize(); ++libIdx)
		{
			ShaderProgramRaytracingLibrary& outLib = outLibs[libIdx];
			const Lib& inLib = libs[libIdx];

			outLib.m_alloc = alloc;

			if(inLib.m_presentStages
			   != (ShaderTypeBit::RAY_GEN | ShaderTypeBit::MISS | ShaderTypeBit::CLOSEST_HIT | ShaderTypeBit::ANY_HIT))
			{
				ANKI_RESOURCE_LOGE("The libray is missing shader shader types: %s", inLib.m_name.cstr());
				return Error::USER_DATA;
			}

			if(inLib.m_rayTypeCount != inLib.m_rayTypeMask.getEnabledBitCount())
			{
				ANKI_RESOURCE_LOGE("Ray types are not contiguous for library: %s", inLib.m_name.cstr());
				return Error::USER_DATA;
			}

			outLib.m_libraryName.create(alloc, inLib.m_name);
			outLib.m_rayTypeCount = inLib.m_rayTypeCount;

			DynamicArrayAuto<RayTracingHitGroup> initInfoHitGroups(alloc);
			DynamicArrayAuto<ShaderPtr> missShaders(alloc);
			DynamicArrayAuto<ShaderPtr> rayGenShaders(alloc);

			// Add the hitgroups to the init info
			for(U32 shaderGroupIdx = 0; shaderGroupIdx < inLib.m_shaderGroups.getSize(); ++shaderGroupIdx)
			{
				const ShaderGroup& inShaderGroup = inLib.m_shaderGroups[shaderGroupIdx];
				ANKI_ASSERT(inShaderGroup.m_hitGroupHash != 0);

				if(inShaderGroup.m_ahit < MAX_U32 || inShaderGroup.m_chit < MAX_U32)
				{
					// Hit shaders

					ANKI_ASSERT(inShaderGroup.m_miss == MAX_U32 && inShaderGroup.m_rayGen == MAX_U32);
					RayTracingHitGroup* infoHitGroup = initInfoHitGroups.emplaceBack();

					if(inShaderGroup.m_ahit < MAX_U32)
					{
						infoHitGroup->m_anyHitShader = inLib.m_shaders[inShaderGroup.m_ahit].m_shader;
					}

					if(inShaderGroup.m_chit < MAX_U32)
					{
						infoHitGroup->m_closestHitShader = inLib.m_shaders[inShaderGroup.m_chit].m_shader;
					}

					// The hit shaders are after ray gen and miss shaders
					const U32 idx =
						inLib.m_rayGenShaderGroupCount + inLib.m_missShaderGroupCount + initInfoHitGroups.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(alloc, inShaderGroup.m_hitGroupHash, idx);
				}
				else if(inShaderGroup.m_miss < MAX_U32)
				{
					// Miss shader

					ANKI_ASSERT(inShaderGroup.m_ahit == MAX_U32 && inShaderGroup.m_chit == MAX_U32
								&& inShaderGroup.m_rayGen == MAX_U32);

					missShaders.emplaceBack(inLib.m_shaders[inShaderGroup.m_miss].m_shader);

					// The miss shaders are after ray gen
					const U32 idx = inLib.m_rayGenShaderGroupCount + missShaders.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(alloc, inShaderGroup.m_hitGroupHash, idx);
				}
				else
				{
					// Ray gen shader

					ANKI_ASSERT(inShaderGroup.m_ahit == MAX_U32 && inShaderGroup.m_chit == MAX_U32
								&& inShaderGroup.m_miss == MAX_U32 && inShaderGroup.m_rayGen < MAX_U32);

					rayGenShaders.emplaceBack(inLib.m_shaders[inShaderGroup.m_rayGen].m_shader);

					// Ray gen shaders are first
					const U32 idx = rayGenShaders.getSize() - 1;
					outLib.m_resourceHashToShaderGroupHandleIndex.emplace(alloc, inShaderGroup.m_hitGroupHash, idx);
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

	return Error::NONE;
}

} // end namespace anki
