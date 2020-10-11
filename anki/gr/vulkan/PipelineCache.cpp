// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/PipelineCache.h>
#include <anki/core/ConfigSet.h>
#include <anki/util/Filesystem.h>
#include <anki/util/File.h>

namespace anki
{

Error PipelineCache::init(VkDevice dev, VkPhysicalDevice pdev, CString cacheDir, const ConfigSet& cfg,
						  GrAllocator<U8> alloc)
{
	ANKI_ASSERT(cacheDir && dev && pdev);
	m_dumpSize = cfg.getNumberU32("gr_diskShaderCacheMaxSize");
	m_dumpFilename.sprintf(alloc, "%s/vk_pipeline_cache", &cacheDir[0]);

	// Try read the pipeline cache file.
	DynamicArrayAuto<U8, PtrSize> diskDump(alloc);
	if(fileExists(m_dumpFilename.toCString()))
	{
		File file;
		ANKI_CHECK(file.open(m_dumpFilename.toCString(), FileOpenFlag::BINARY | FileOpenFlag::READ));

		PtrSize diskDumpSize = file.getSize();
		if(diskDumpSize <= sizeof(U8) * VK_UUID_SIZE)
		{
			ANKI_VK_LOGI("Pipeline cache dump appears to be empty: %s", &m_dumpFilename[0]);
		}
		else
		{
			// Get current pipeline UUID and compare it with the cache's
			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(pdev, &props);

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

	ANKI_VK_CHECK(vkCreatePipelineCache(dev, &ci, nullptr, &m_cacheHandle));

	return Error::NONE;
}

void PipelineCache::destroy(VkDevice dev, VkPhysicalDevice pdev, GrAllocator<U8> alloc)
{
	Error err = destroyInternal(dev, pdev, alloc);
	if(err)
	{
		ANKI_VK_LOGE("An error occurred while storing the pipeline cache to disk. Will ignore");
	}
}

Error PipelineCache::destroyInternal(VkDevice dev, VkPhysicalDevice pdev, GrAllocator<U8> alloc)
{
	if(m_cacheHandle)
	{
		ANKI_ASSERT(dev && pdev);

		// Get size of cache
		size_t size = 0;
		ANKI_VK_CHECK(vkGetPipelineCacheData(dev, m_cacheHandle, &size, nullptr));
		size = min(size, m_dumpSize);

		if(size > 0)
		{
			// Read cache
			DynamicArrayAuto<U8, PtrSize> cacheData(alloc);
			cacheData.create(size);
			ANKI_VK_CHECK(vkGetPipelineCacheData(dev, m_cacheHandle, &size, &cacheData[0]));

			// Write file
			File file;
			ANKI_CHECK(file.open(&m_dumpFilename[0], FileOpenFlag::BINARY | FileOpenFlag::WRITE));

			VkPhysicalDeviceProperties props;
			vkGetPhysicalDeviceProperties(pdev, &props);

			ANKI_CHECK(file.write(&props.pipelineCacheUUID[0], VK_UUID_SIZE));
			ANKI_CHECK(file.write(&cacheData[0], size));

			ANKI_VK_LOGI("Dumped %zu bytes of the pipeline cache", size);
		}

		// Destroy cache
		vkDestroyPipelineCache(dev, m_cacheHandle, nullptr);
		m_cacheHandle = VK_NULL_HANDLE;
	}

	m_dumpFilename.destroy(alloc);

	return Error::NONE;
}

} // end namespace anki
