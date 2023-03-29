// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Scene/SceneNode.h>

namespace anki {

static SceneComponentRtti* g_rttis[kMaxSceneComponentClasses] = {};
SceneComponentCallbacks g_sceneComponentCallbacks;
static U32 g_sceneComponentClassCount = 0;

SceneComponentRtti::SceneComponentRtti(const char* name, F32 updateWeight, U32 size, U32 alignment,
									   SceneComponentVtable vtable)
{
	if(g_sceneComponentClassCount >= kMaxSceneComponentClasses)
	{
		// No special logging because this function is called before main
		printf("Reached maximum component count. Increase kMaxSceneComponentClasses\n");
		exit(-1);
	}

	m_updateWeight = updateWeight;

	m_className = name;
	ANKI_ASSERT(size < getMaxNumericLimit<decltype(m_classSize)>());
	m_classSize = decltype(m_classSize)(size);

	ANKI_ASSERT(alignment < getMaxNumericLimit<decltype(m_classAlignment)>());
	m_classAlignment = decltype(m_classAlignment)(alignment);

	m_classId = kMaxU8;

	g_rttis[g_sceneComponentClassCount] = this;

#define ANKI_SCENE_COMPONENT_VIRTUAL(name, type) \
	g_sceneComponentCallbacks.m_##name[g_sceneComponentClassCount] = vtable.m_##name;
#include <AnKi/Scene/Components/SceneComponentVirtuals.defs.h>

	++g_sceneComponentClassCount;

	// Sort everything because the IDs should be consistend between platforms and compilation builds
	{
		// Copy to a temp array
		class Temp
		{
		public:
			SceneComponentRtti* m_rrti;
			SceneComponentVtable m_vtable;
		};

		Temp temps[kMaxSceneComponentClasses];
		for(U32 i = 0; i < g_sceneComponentClassCount; ++i)
		{
			temps[i].m_rrti = g_rttis[i];
			temps[i].m_vtable = {
#define ANKI_SCENE_COMPONENT_VIRTUAL(name, type) g_sceneComponentCallbacks.m_##name[i]
#define ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR ,
#include <AnKi/Scene/Components/SceneComponentVirtuals.defs.h>
			};
		}

		std::sort(&temps[0], &temps[g_sceneComponentClassCount], [](const Temp& a, const Temp& b) {
			return std::strcmp(a.m_rrti->m_className, b.m_rrti->m_className) < 0;
		});

		// Re-calculate the glass IDs
		for(U32 i = 0; i < g_sceneComponentClassCount; ++i)
		{
			temps[i].m_rrti->m_classId = U8(i);
		}

		// Copy back
		for(U32 i = 0; i < g_sceneComponentClassCount; ++i)
		{
			g_rttis[i] = temps[i].m_rrti;
#define ANKI_SCENE_COMPONENT_VIRTUAL(name, type) g_sceneComponentCallbacks.m_##name[i] = temps[i].m_vtable.m_##name;
#include <AnKi/Scene/Components/SceneComponentVirtuals.defs.h>
		}
	}
}

SceneComponent::SceneComponent([[maybe_unused]] SceneNode* node, U8 classId)
	: m_classId(classId)
{
	ANKI_ASSERT(classId < g_sceneComponentClassCount);
}

const SceneComponentRtti& SceneComponent::findClassRtti(CString className)
{
	for(U32 i = 0; i < g_sceneComponentClassCount; ++i)
	{
		ANKI_ASSERT(g_rttis[i]);
		ANKI_ASSERT(g_rttis[i]->m_className);
		if(g_rttis[i]->m_className == className)
		{
			return *g_rttis[i];
		}
	}

	ANKI_ASSERT(0);
	return *g_rttis[0];
}

const SceneComponentRtti& SceneComponent::getClassRtti(U8 classId)
{
	ANKI_ASSERT(classId < g_sceneComponentClassCount);
	ANKI_ASSERT(g_rttis[classId]);
	return *g_rttis[classId];
}

} // namespace anki
