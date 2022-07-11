// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/Vulkan/VulkanObject.h>
#include <AnKi/Gr/GrManager.h>
#include <AnKi/Gr/Vulkan/GrManagerImpl.h>

#include <AnKi/Gr/Vulkan/BufferImpl.h>
#include <AnKi/Gr/Vulkan/TextureImpl.h>
#include <AnKi/Gr/Vulkan/SamplerImpl.h>
#include <AnKi/Gr/Vulkan/ShaderImpl.h>
#include <AnKi/Gr/Vulkan/ShaderProgramImpl.h>
#include <AnKi/Gr/Vulkan/CommandBufferImpl.h>
#include <AnKi/Gr/Vulkan/FramebufferImpl.h>
#include <AnKi/Gr/Vulkan/OcclusionQueryImpl.h>
#include <AnKi/Gr/Vulkan/TimestampQueryImpl.h>
#include <AnKi/Gr/Vulkan/FenceImpl.h>
#include <AnKi/Gr/Vulkan/AccelerationStructureImpl.h>
#include <AnKi/Gr/Vulkan/GrUpscalerImpl.h>

namespace anki {

#define ANKI_INSTANTIATE_GR_OBJECT(type_) \
	template<> \
	VkDevice VulkanObject<type_, type_##Impl>::getDevice() const \
	{ \
		return getGrManagerImpl().getDevice(); \
	} \
	template<> \
	GrManagerImpl& VulkanObject<type_, type_##Impl>::getGrManagerImpl() \
	{ \
		return static_cast<GrManagerImpl&>(static_cast<type_##Impl*>(this)->getManager()); \
	} \
	template<> \
	const GrManagerImpl& VulkanObject<type_, type_##Impl>::getGrManagerImpl() const \
	{ \
		return static_cast<const GrManagerImpl&>(static_cast<const type_##Impl*>(this)->getManager()); \
	}

#define ANKI_INSTANTIATE_GR_OBJECT_DELIMITER()
#include <AnKi/Gr/Utils/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_GR_OBJECT_DELIMITER
#undef ANKI_INSTANTIATE_GR_OBJECT

} // end namespace anki
