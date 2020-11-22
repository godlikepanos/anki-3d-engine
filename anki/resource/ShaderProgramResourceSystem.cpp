// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/resource/ShaderProgramResourceSystem.h>
#include <anki/resource/ResourceFilesystem.h>
#include <anki/util/Tracer.h>
#include <anki/gr/GrManager.h>
#include <anki/shader_compiler/ShaderProgramCompiler.h>
#include <anki/util/Filesystem.h>
#include <anki/util/ThreadHive.h>
#include <anki/util/System.h>

namespace anki
{

ShaderProgramResourceSystem::~ShaderProgramResourceSystem()
{
	m_cacheDir.destroy(m_alloc);

	for(ShaderProgramRaytracingLibrary& lib : m_rtLibraries)
	{
		lib.m_libraryName.destroy(m_alloc);
		lib.m_groupHashToGroupIndex.destroy(m_alloc);
	}

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

	ThreadHive threadHive(getCpuCoresCount(), alloc, false);

	// Compute hash for both
	const GpuDeviceCapabilities caps = gr.getDeviceCapabilities();
	const BindlessLimits limits = gr.getBindlessLimits();
	U64 gpuHash = computeHash(&caps, sizeof(caps));
	gpuHash = appendHash(&limits, sizeof(limits), gpuHash);
	gpuHash = appendHash(&SHADER_BINARY_VERSION, sizeof(SHADER_BINARY_VERSION), gpuHash);

	ANKI_CHECK(fs.iterateAllFilenames([&](CString fname) -> Error {
		// Check file extension
		StringAuto extension(alloc);
		getFilepathExtension(fname, extension);
		if(extension.getLength() != 8 || extension != "ankiprog")
		{
			return Error::NONE;
		}

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
		ANKI_CHECK(compileShaderProgram(fname, fsystem, &skip, &taskManager, alloc, caps, limits, binary));

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

	ANKI_RESOURCE_LOGI("Compiled %u shader programs", shadersCompileCount);
	return Error::NONE;
}

Error ShaderProgramResourceSystem::createRayTracingPrograms(CString cacheDir, const StringListAuto& rtProgramFilenames,
															GrManager& gr, GenericMemoryPoolAllocator<U8>& alloc,
															DynamicArray<ShaderProgramRaytracingLibrary>& outLibs)
{
	ANKI_RESOURCE_LOGI("Creating ray tracing programs");
	U32 rtProgramCount = 0;

	// Group things together
	class Shader
	{
	public:
		ShaderPtr m_shader;
		U64 m_hash = 0;
	};

	class HitGroup
	{
	public:
		U32 m_chit = MAX_U32;
		U32 m_ahit = MAX_U32;
		U64 m_hitGroupHash = 0;
	};

	class RayType
	{
	public:
		RayType(GenericMemoryPoolAllocator<U8> alloc)
			: m_hitGroups(alloc)
		{
		}

		U32 m_miss = MAX_U32;
		U32 m_typeIndex = MAX_U32;
		DynamicArrayAuto<HitGroup> m_hitGroups;
	};

	class Lib
	{
	public:
		Lib(GenericMemoryPoolAllocator<U8> alloc)
			: m_alloc(alloc)
		{
		}

		GenericMemoryPoolAllocator<U8> m_alloc;
		StringAuto m_name{m_alloc};
		ShaderPtr m_rayGenShader;
		DynamicArrayAuto<Shader> m_shaders{m_alloc};
		DynamicArrayAuto<RayType> m_rayTypes{m_alloc};
		ShaderTypeBit m_presentStages = ShaderTypeBit::NONE;
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

		const U32 rayTypeNumber = binary.m_rayType;

		// Create the program name
		StringAuto progName(alloc);
		getFilepathFilename(filename, progName);
		char* cprogName = const_cast<char*>(progName.cstr());
		if(progName.getLength() > MAX_GR_OBJECT_NAME_LENGTH)
		{
			cprogName[MAX_GR_OBJECT_NAME_LENGTH] = '\0';
		}

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
				libs.emplaceBack(alloc);
				lib = &libs.getBack();
				lib->m_name.create(CString(&binary.m_libraryName[0]));
			}
		}

		// Ray gen
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::RAY_GEN))
		{
			if(lib->m_rayGenShader.isCreated())
			{
				ANKI_RESOURCE_LOGE("The library already has a ray gen shader: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::RAY_GEN))
			{
				ANKI_RESOURCE_LOGE("Ray gen can't co-exist with other types: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(binary.m_constants.getSize() || binary.m_mutators.getSize())
			{
				ANKI_RESOURCE_LOGE("Ray gen can't have spec constants or mutators ATM: %s", filename.cstr());
				return Error::USER_DATA;
			}

			ShaderInitInfo inf(cprogName);
			inf.m_shaderType = ShaderType::RAY_GEN;
			inf.m_binary = binary.m_codeBlocks[0].m_binary;
			lib->m_rayGenShader = gr.newShader(inf);

			lib->m_presentStages |= ShaderTypeBit::RAY_GEN;
		}

		// Miss shaders
		if(!!(binary.m_presentShaderTypes & ShaderTypeBit::MISS))
		{
			if(!!(binary.m_presentShaderTypes & ~ShaderTypeBit::MISS))
			{
				ANKI_RESOURCE_LOGE("Miss shaders can't co-exist with other types: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(binary.m_constants.getSize() || binary.m_mutators.getSize())
			{
				ANKI_RESOURCE_LOGE("Miss can't have spec constants or mutators ATM: %s", filename.cstr());
				return Error::USER_DATA;
			}

			if(rayTypeNumber == MAX_U32)
			{
				ANKI_RESOURCE_LOGE("Miss shader should have set the ray type: %s", filename.cstr());
				return Error::USER_DATA;
			}

			RayType* rayType = nullptr;
			for(RayType& rt : lib->m_rayTypes)
			{
				if(rt.m_typeIndex == rayTypeNumber)
				{
					rayType = &rt;
					break;
				}
			}

			if(rayType == nullptr)
			{
				lib->m_rayTypes.emplaceBack(alloc);
				rayType = &lib->m_rayTypes.getBack();
				rayType->m_typeIndex = rayTypeNumber;
			}

			if(rayType->m_miss != MAX_U32)
			{
				ANKI_RESOURCE_LOGE(
					"There is another miss program with the same library and sub-library names with this: %s",
					filename.cstr());
				return Error::USER_DATA;
			}

			Shader* shader = nullptr;
			for(Shader& s : lib->m_shaders)
			{
				if(s.m_hash == binary.m_codeBlocks[0].m_hash)
				{
					shader = &s;
					break;
				}
			}

			if(shader == nullptr)
			{
				shader = lib->m_shaders.emplaceBack();

				ShaderInitInfo inf(cprogName);
				inf.m_shaderType = ShaderType::MISS;
				inf.m_binary = binary.m_codeBlocks[0].m_binary;
				shader->m_shader = gr.newShader(inf);
				shader->m_hash = binary.m_codeBlocks[0].m_hash;
			}

			rayType->m_miss = U32(shader - &lib->m_shaders[0]);

			lib->m_presentStages |= ShaderTypeBit::MISS;
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
			if(binary.m_mutations.getSize())
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

				// Generate the hash
				const U64 hitGroupHash =
					ShaderProgramRaytracingLibrary::generateHitGroupHash(filename, mutation.m_hash);

				HitGroup hitGroup;
				hitGroup.m_hitGroupHash = hitGroupHash;

				for(ShaderType shaderType : EnumIterable<ShaderType>())
				{
					const U32 codeBlockIndex = variant.m_codeBlockIndices[shaderType];
					if(codeBlockIndex == MAX_U32)
					{
						continue;
					}

					ANKI_ASSERT(shaderType == ShaderType::ANY_HIT || shaderType == ShaderType::CLOSEST_HIT);

					// Find the shader
					Shader* shader = nullptr;
					for(Shader& s : lib->m_shaders)
					{
						if(s.m_hash == binary.m_codeBlocks[codeBlockIndex].m_hash)
						{
							shader = &s;
							break;
						}
					}

					// Crete the shader
					if(shader == nullptr)
					{
						shader = lib->m_shaders.emplaceBack();

						ShaderInitInfo inf(cprogName);
						inf.m_shaderType = shaderType;
						inf.m_binary = binary.m_codeBlocks[codeBlockIndex].m_binary;
						shader->m_shader = gr.newShader(inf);
						shader->m_hash = binary.m_codeBlocks[codeBlockIndex].m_hash;
					}

					const U32 shaderIndex = U32(shader - &lib->m_shaders[0]);

					if(shaderType == ShaderType::ANY_HIT)
					{
						hitGroup.m_ahit = shaderIndex;
						lib->m_presentStages |= ShaderTypeBit::ANY_HIT;
					}
					else
					{
						hitGroup.m_chit = shaderIndex;
						lib->m_presentStages |= ShaderTypeBit::CLOSEST_HIT;
					}
				}

				// Get or create the ray type
				RayType* rayType = nullptr;
				for(RayType& rt : lib->m_rayTypes)
				{
					if(rt.m_typeIndex == rayTypeNumber)
					{
						rayType = &rt;
						break;
					}
				}

				if(rayType == nullptr)
				{
					lib->m_rayTypes.emplaceBack(alloc);
					rayType = &lib->m_rayTypes.getBack();
					rayType->m_typeIndex = rayTypeNumber;
				}

				// Try to find if the hit group aleady exists. If it does then something is wrong
				for(const HitGroup& hg : rayType->m_hitGroups)
				{
					if(hg.m_hitGroupHash == hitGroup.m_hitGroupHash)
					{
						ANKI_ASSERT(!"Found a hitgroup with the same hash. Something is wrong");
					}
				}

				// Create the hitgroup
				rayType->m_hitGroups.emplaceBack(hitGroup);
			}
		}
	}

	// Create the libraries
	if(libs.getSize() != 0)
	{
		outLibs.resize(alloc, libs.getSize());

		for(U32 libIdx = 0; libIdx < libs.getSize(); ++libIdx)
		{
			ShaderProgramRaytracingLibrary& outLib = outLibs[libIdx];
			Lib& inLib = libs[libIdx];

			if(inLib.m_presentStages
			   != (ShaderTypeBit::RAY_GEN | ShaderTypeBit::MISS | ShaderTypeBit::CLOSEST_HIT | ShaderTypeBit::ANY_HIT))
			{
				ANKI_RESOURCE_LOGE("The libray is missing shader shader types: %s", inLib.m_name.cstr());
				return Error::USER_DATA;
			}

			// Sort because the expectation is that the miss shaders are organized based on ray type
			std::sort(inLib.m_rayTypes.getBegin(), inLib.m_rayTypes.getEnd(),
					  [](const RayType& a, const RayType& b) { return a.m_typeIndex < b.m_typeIndex; });

			outLib.m_libraryName.create(alloc, inLib.m_name);
			outLib.m_rayTypeCount = inLib.m_rayTypes.getSize();

			DynamicArrayAuto<RayTracingHitGroup> initInfoHitGroups(alloc);
			DynamicArrayAuto<ShaderPtr> missShaders(alloc);

			for(U32 rayTypeIdx = 0; rayTypeIdx < inLib.m_rayTypes.getSize(); ++rayTypeIdx)
			{
				const RayType& inRayType = inLib.m_rayTypes[rayTypeIdx];

				if(inRayType.m_typeIndex != rayTypeIdx)
				{
					ANKI_RESOURCE_LOGE("Ray types are not contiguous for library: %s", inLib.m_name.cstr());
					return Error::USER_DATA;
				}

				// Add the hitgroups to the init info
				for(U32 hitGroupIdx = 0; hitGroupIdx < inRayType.m_hitGroups.getSize(); ++hitGroupIdx)
				{
					const HitGroup& inHitGroup = inRayType.m_hitGroups[hitGroupIdx];

					outLib.m_groupHashToGroupIndex.emplace(alloc, inHitGroup.m_hitGroupHash,
														   initInfoHitGroups.getSize() + outLib.m_rayTypeCount + 1);

					RayTracingHitGroup* infoHitGroup = initInfoHitGroups.emplaceBack();
					if(inHitGroup.m_ahit != MAX_U32)
					{
						infoHitGroup->m_anyHitShader = inLib.m_shaders[inHitGroup.m_ahit].m_shader;
					}

					if(inHitGroup.m_chit != MAX_U32)
					{
						infoHitGroup->m_closestHitShader = inLib.m_shaders[inHitGroup.m_chit].m_shader;
					}
				}

				// Add the miss shader
				ANKI_ASSERT(inRayType.m_miss != MAX_U32);
				missShaders.emplaceBack(inLib.m_shaders[inRayType.m_miss].m_shader);
			}

			// Program name
			StringAuto progName(alloc, inLib.m_name);
			char* cprogName = const_cast<char*>(progName.cstr());
			if(progName.getLength() > MAX_GR_OBJECT_NAME_LENGTH)
			{
				cprogName[MAX_GR_OBJECT_NAME_LENGTH] = '\0';
			}

			// Create the program
			ShaderProgramInitInfo inf(cprogName);
			inf.m_rayTracingShaders.m_rayGenShader = inLib.m_rayGenShader;
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
