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

Error ShaderProgramResourceSystem::compileAllShaders(CString cacheDir, GrManager& gr, ResourceFilesystem& fs,
													 GenericMemoryPoolAllocator<U8>& alloc)
{
	ANKI_TRACE_SCOPED_EVENT(COMPILE_SHADERS);
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
			return Error::NONE;
		}

		// Get some filenames
		StringAuto baseFname(alloc);
		getFilepathFilename(fname, baseFname);
		StringAuto metaFname(alloc);
		metaFname.sprintf("%s/%smeta", cacheDir.cstr(), baseFname.cstr());

		// Get the hash from the meta file
		U64 metafileHash = 0;
		if(fileExists(metaFname))
		{
			File metaFile;
			ANKI_CHECK(metaFile.open(metaFname, FileOpenFlag::READ | FileOpenFlag::BINARY));
			ANKI_CHECK(metaFile.read(&metafileHash, sizeof(metafileHash)));
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
			ANKI_CHECK(metaFile.write(&skip.m_newHash, sizeof(skip.m_newHash)));
		}

		// Save the binary to the cache
		if(!cachedBinIsUpToDate)
		{
			StringAuto storeFname(alloc);
			storeFname.sprintf("%s/%sbin", cacheDir.cstr(), baseFname.cstr());
			ANKI_CHECK(binary.serializeToFile(storeFname));
		}

		return Error::NONE;
	}));

	ANKI_RESOURCE_LOGI("Compiled %u shader programs", shadersCompileCount);
	return Error::NONE;
}

} // end namespace anki
