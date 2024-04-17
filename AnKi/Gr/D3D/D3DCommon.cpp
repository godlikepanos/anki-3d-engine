// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Gr/D3D/D3DCommon.h>
#include <AnKi/Gr/D3D/D3DGrManager.h>

namespace anki {

ID3D12Device& getDevice()
{
	return static_cast<GrManagerImpl&>(GrManager::getSingleton()).getDevice();
}

GrManagerImpl& getGrManagerImpl()
{
	return static_cast<GrManagerImpl&>(GrManager::getSingleton());
}

} // end namespace anki
