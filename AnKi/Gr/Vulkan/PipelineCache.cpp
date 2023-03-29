// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/PipelineCache.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>
#include <AnKi/Core/ConfigSet.h>
#include <AnKi/Util/Filesystem.h>
#include <AnKi/Util/File.h>

namespace anki {

Error PipelineCache::init(CString cacheDir)
{
	ANKI_ASSERT(cacheDir);
	m_dumpSize = ConfigSet::getSingleton().getGrDiskShaderCacheMaxSize();
	m_dumpFilename.sprintf("%s/VkPipelineCache", cacheDir.cstr());

	// Try read the pipeline cache file.
	GrDynamicArray<U8, PtrSize> diskDump;
	if(fileExists(m_dumpFilename.toCString()))
	{
		File file;
		ANKI_CHECK(file.open(m_dumpFilename.toCString(), FileOpenFlag::kBinary | FileOpenFlag::kRead));

		const PtrSize diskDumpSize = file.getSize();
		if(diskDumpSize <= sizeof(U8) * VK_UUID_SIZE)
		{
			ANKI_VK_LOGI("Pipeline cache dump appears to be empty: %s", &m_dumpFilename[0]);
		}
		else
		{
			// Get current pipeline UUID and compare it with the cache's
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(getGrManagerImpl().getPhysicalDevice(), &props);

			Array<U8, VK_UUID_SIZE> cacheUuid;
			ANKI_CHECK(file.read(&cacheUuid[0], VK_UUID_SIZE));

			if(memcmp(&cacheUuid[0], &props.pipelineCacheUUID[0], VK_UUID_SIZE) != 0)
			{
				ANKI_VK_LOGI("Pipeline cache dump is not compatible with the current device: %s", &m_dumpFilename[0]);
			}
			else
			{
				diskDump.create(diskDumpSize - VK_UUID_SIZE);
				ANKI_CHECK(file.read(&diskDump[0], diskDumpSize - VK_UUID_SIZE));
			}
		}
	}
	else
	{
		ANKI_VK_LOGI("Pipeline cache dump not found: %s", &m_dumpFilename[0]);
	}

	// Create the cache
	VkPipelineCacheCreateInfo ci = {};
	ci.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if(diskDump.getSize())
	{
		ANKI_VK_LOGI("Will load %zu bytes of pipeline cache", diskDump.getSize());
		ci.initialDataSize = diskDump.getSize();
		ci.pInitialData = &diskDump[0];
	}

	ANKI_VK_CHECK(vkCreatePipelineCache(getVkDevice(), &ci, nullptr, &m_cacheHandle));

	return Error::kNone;
}

void PipelineCache::destroy()
{
	const Error err = destroyInternal();
	if(err)
	{
		ANKI_VK_LOGE("An error occurred while storing the pipeline cache to disk. Will ignore");
	}

	m_dumpFilename.destroy();
}

Error PipelineCache::destroyInternal()
{
	if(m_cacheHandle)
	{
		// Get size of cache
		size_t size = 0;
		ANKI_VK_CHECK(vkGetPipelineCacheData(getVkDevice(), m_cacheHandle, &size, nullptr));
		size = min(size, m_dumpSize);

		if(size > 0)
		{
			// Read cache
			GrDynamicArray<U8, PtrSize> cacheData;
			cacheData.create(size);
			ANKI_VK_CHECK(vkGetPipelineCacheData(getVkDevice(), m_cacheHandle, &size, &cacheData[0]));

			// Write file
			File file;
			ANKI_CHECK(file.open(&m_dumpFilename[0], FileOpenFlag::kBinary | FileOpenFlag::kWrite));

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(getGrManagerImpl().getPhysicalDevice(), &props);

			ANKI_CHECK(file.write(&props.pipelineCacheUUID[0], VK_UUID_SIZE));
			ANKI_CHECK(file.write(&cacheData[0], size));

			ANKI_VK_LOGI("Dumped %zu bytes of the pipeline cache", size);
		}

		// Destroy cache
		vkDestroyPipelineCache(getVkDevice(), m_cacheHandle, nullptr);
		m_cacheHandle = VK_NULL_HANDLE;
	}

	return Error::kNone;
}

} // end namespace anki
