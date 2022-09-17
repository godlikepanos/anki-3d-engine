// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Common.h>
#include <AnKi/Util/Functions.h>
#include <AnKi/Util/BitMask.h>

namespace anki {

/// @addtogroup scene
/// @{

/// Scene component class info.
class SceneComponentRtti
{
public:
	using Constructor = void (*)(SceneComponent*, SceneNode*);

	U8 m_classId;
	const char* m_className;
	U32 m_classSize;
	U32 m_classAlignment;
	Constructor m_constructorCallback;

	SceneComponentRtti(const char* name, U32 size, U32 alignment, Constructor constructor);
};

/// Define a scene component.
#define ANKI_SCENE_COMPONENT(className) \
	static SceneComponentRtti _m_rtti; \
	static void _construct(SceneComponent* self, SceneNode* node) \
	{ \
		::new(self) className(node); \
	} \
\
public: \
	static U8 getStaticClassId() \
	{ \
		return _m_rtti.m_classId; \
	} \
\
private:

/// Define the statics of a scene component.
#define ANKI_SCENE_COMPONENT_STATICS(className) \
	SceneComponentRtti className::_m_rtti(ANKI_STRINGIZE(className), sizeof(className), alignof(className), \
										  className::_construct);

/// Passed to SceneComponent::update.
/// @memberof SceneComponent
class SceneComponentUpdateInfo
{
public:
	SceneNode* m_node;
	const Second m_previousTime;
	const Second m_currentTime;
	const Second m_dt;

	SceneComponentUpdateInfo(Second prevTime, Second crntTime)
		: m_previousTime(prevTime)
		, m_currentTime(crntTime)
		, m_dt(crntTime - prevTime)
	{
	}
};

/// Scene node component
class SceneComponent
{
public:
	/// Construct the scene component.
	SceneComponent(SceneNode* node, U8 classId, Bool isFeedbackComponent = false);

	virtual ~SceneComponent()
	{
	}

	U8 getClassId() const
	{
		return m_classId;
	}

	Timestamp getTimestamp() const
	{
		return m_timestamp;
	}

	Bool isFeedbackComponent() const
	{
		return m_feedbackComponent;
	}

	/// Do some updating
	/// @param[in,out] info Update info.
	/// @param[out] updated true if an update happened.
	virtual Error update([[maybe_unused]] SceneComponentUpdateInfo& info, Bool& updated)
	{
		updated = false;
		return Error::kNone;
	}

	/// Don't call it.
	void setTimestamp(Timestamp timestamp)
	{
		ANKI_ASSERT(timestamp > 0);
		ANKI_ASSERT(timestamp >= m_timestamp);
		m_timestamp = timestamp;
	}

	static const SceneComponentRtti& findClassRtti(CString className);

	static const SceneComponentRtti& findClassRtti(U8 classId);

private:
	Timestamp m_timestamp = 1; ///< Indicates when an update happened
	U8 m_classId : 7; ///< Cache the type ID.
	U8 m_feedbackComponent : 1;
};
/// @}

} // end namespace anki
