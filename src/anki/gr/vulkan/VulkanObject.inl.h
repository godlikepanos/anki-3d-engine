// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/gr/vulkan/VulkanObject.h>

namespace anki
{

template<typename TBaseClass, typename TImplClass>
template<typename TGrManager, typename... TArgs>
inline TBaseClass* VulkanObject<TBaseClass, TImplClass>::newInstanceHelper(TGrManager* manager, TArgs&&... args)
{
	TImplClass* impl = manager->getAllocator().template newInstance<TImplClass>(manager);
	Error err = impl->init(std::forward<TArgs>(args)...);
	if(err)
	{
		manager->getAllocator().deleteInstance(impl);
		impl = nullptr;
		ANKI_VK_LOGF("Error while creating an instance. Will not try to recover");
	}

	return impl;
}

} // end namespace anki