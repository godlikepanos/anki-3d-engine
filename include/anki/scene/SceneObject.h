// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SCENE_OBJECT_H
#define ANKI_SCENE_SCENE_OBJECT_H

#include "anki/scene/Common.h"
#include "anki/util/Object.h"
#include "anki/util/Functions.h"
#include "anki/util/Enum.h"

namespace anki {

// Forward
class SceneGraph;
class SceneObject;

/// @addtogroup scene
/// @{

/// The callbacks of SceneObject
struct SceneObjectCallbackCollection
{
	/// Called when a child is been removed from a parent
	void onChildRemoved(SceneObject* child, SceneObject* parent);

	/// Called when a child is been added to a parent
	void onChildAdded(SceneObject* child, SceneObject* parent)
	{
		ANKI_ASSERT(child && parent);
		// Do nothing
	}
};

/// The base of all scene related objects
class SceneObject: 
	public Object<SceneObject, SceneAllocator<SceneObject>, 
	SceneObjectCallbackCollection>
{
public:
	using Base = Object<SceneObject, SceneAllocator<SceneObject>,
		SceneObjectCallbackCollection>;

	enum class Type: U8
	{
		NONE = 0,
		SCENE_NODE = 1 << 0,
		EVENT = 1 << 1,
		_TYPE_MASK = SCENE_NODE | EVENT,
		_MARKED_FOR_DELETION = 1 << 2
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Type, friend)

	SceneObject(Type type, SceneGraph* scene);

	virtual ~SceneObject();

	Type getType() const
	{
		return m_bits & Type::_TYPE_MASK;
	}

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	ANKI_USE_RESULT Error addChild(SceneObject* obj)
	{
		return Base::addChild(getSceneAllocator(), obj);
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	Bool isMarkedForDeletion() const
	{
		return (m_bits & Type::_MARKED_FOR_DELETION) != Type::NONE;
	}

	void markForDeletion();

	/// Downcast the class
	template<typename TScObj>
	TScObj& downCast()
	{
		ANKI_ASSERT(TScObj::getClassType() == getType());
		TScObj* out = staticCastPtr<TScObj*>(this);
		return *out;
	}

	/// Downcast the class
	template<typename TScObj>
	const TScObj& downCast() const
	{
		ANKI_ASSERT(TScObj::getClassType() == getType());
		const TScObj* out = staticCastPtr<const TScObj*>(this);
		return *out;
	}

private:
	SceneGraph* m_scene = 0;
	Type m_bits = Type::NONE; ///< Contains the type and other flags
};

inline void SceneObjectCallbackCollection::onChildRemoved(
	SceneObject* child, SceneObject* parent)
{
	child->markForDeletion();
}

/// @}

} // end namespace anki

#endif
