// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/VulkanObject.h>
#include <anki/gr/GrManager.h>
#include <anki/gr/vulkan/GrManagerImpl.h>

#include <anki/gr/vulkan/BufferImpl.h>
#include <anki/gr/vulkan/TextureImpl.h>
#include <anki/gr/vulkan/SamplerImpl.h>
#include <anki/gr/vulkan/ShaderImpl.h>
#include <anki/gr/vulkan/ShaderProgramImpl.h>
#include <anki/gr/vulkan/CommandBufferImpl.h>
#include <anki/gr/vulkan/FramebufferImpl.h>
#include <anki/gr/vulkan/OcclusionQueryImpl.h>
#include <anki/gr/vulkan/TimestampQueryImpl.h>
#include <anki/gr/vulkan/FenceImpl.h>
#include <anki/gr/vulkan/AccelerationStructureImpl.h>

namespace anki
{

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
#include <anki/gr/utils/InstantiationMacros.h>
#undef ANKI_INSTANTIATE_GR_OBJECT_DELIMITER
#undef ANKI_INSTANTIATE_GR_OBJECT

} // end namespace anki
