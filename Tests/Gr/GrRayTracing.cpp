// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <Tests/Framework/Framework.h>
#include <Tests/Gr/GrCommon.h>
#include <AnKi/Gr.h>
#include <AnKi/Window/NativeWindow.h>
#include <AnKi/Window/Input.h>
#include <AnKi/Util/CVarSet.h>
#include <AnKi/GpuMemory/RebarTransientMemoryPool.h>
#include <AnKi/Util/HighRezTimer.h>
#include <AnKi/Resource/TransferGpuAllocator.h>
#include <AnKi/ShaderCompiler/ShaderParser.h>
#include <AnKi/Collision/Aabb.h>
#include <AnKi/Util/WeakArray.h>
#include <ctime>

using namespace anki;

ANKI_TEST(Gr, EmptyAs)
{
	g_cvarGrRayTracing = true;
	commonInit();

	{
		AccelerationStructureInitInfo init;
		init.m_type = AccelerationStructureType::kTopLevel;

		GrManager::getSingleton().getAccelerationStructureMemoryRequirement(init);
	}

	{
		AccelerationStructureInitInfo init;
		init.m_type = AccelerationStructureType::kTopLevel;

		AccelerationStructurePtr as = GrManager::getSingleton().newAccelerationStructure(init);
	}

	commonDestroy();
}
