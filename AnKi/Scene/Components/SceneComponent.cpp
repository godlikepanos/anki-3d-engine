// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <AnKi/Scene/Components/SceneComponent.h>

namespace anki {

constexpr U32 kMaxSceneComponentClasses = 64;
static_assert(kMaxSceneComponentClasses < 128, "It can oly be 7 bits because of SceneComponent::m_classId");
static SceneComponentRtti* g_rttis[kMaxSceneComponentClasses] = {};
static U32 g_rttiCount = 0;

SceneComponentRtti::SceneComponentRtti(const char* name, U32 size, U32 alignment, Constructor constructor)
{
	if(g_rttiCount >= kMaxSceneComponentClasses)
	{
		// No special logging because this function is called before main
		printf("Reached maximum component count. Increase kMaxSceneComponentClasses\n");
		exit(-1);
	}

	m_constructorCallback = constructor;
	m_className = name;
	m_classSize = size;
	m_classAlignment = alignment;
	m_classId = kMaxU8;

	g_rttis[g_rttiCount++] = this;

	// Sort everything because the IDs should be consistend between platforms and compilation builds
	std::sort(&g_rttis[0], &g_rttis[g_rttiCount], [](const SceneComponentRtti* a, const SceneComponentRtti* b) {
		return std::strcmp(a->m_className, b->m_className) < 0;
	});

	// Re-calculate the glass IDs
	for(U32 i = 0; i < g_rttiCount; ++i)
	{
		g_rttis[i]->m_classId = U8(i);
	}
}

SceneComponent::SceneComponent([[maybe_unused]] SceneNode* node, U8 classId, Bool isFeedbackComponent)
	: m_classId(classId & 0x7F)
	, m_feedbackComponent(isFeedbackComponent)
{
	ANKI_ASSERT(classId < g_rttiCount);
}

const SceneComponentRtti& SceneComponent::findClassRtti(CString className)
{
	for(U32 i = 0; i < g_rttiCount; ++i)
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

const SceneComponentRtti& SceneComponent::findClassRtti(U8 classId)
{
	ANKI_ASSERT(classId < g_rttiCount);
	ANKI_ASSERT(g_rttis[classId]);
	return *g_rttis[classId];
}

} // namespace anki
