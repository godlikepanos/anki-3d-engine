// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_SCENE_OBJECT_H
#define ANKI_SCENE_SCENE_OBJECT_H

#include "anki/scene/Common.h"
#include "anki/util/Object.h"
#include "anki/util/Functions.h"

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
	enum Type
	{
		SCENE_NODE_TYPE = 0,
		EVENT_TYPE = 1 << 0
	};

	typedef Object<SceneObject, SceneAllocator<SceneObject>,
		SceneObjectCallbackCollection> Base;

	SceneObject(Type type, SceneObject* parent, SceneGraph* scene);

	virtual ~SceneObject();

	Type getType() const
	{
		return (Type)(flags & EVENT_TYPE);
	}

	SceneAllocator<U8> getSceneAllocator() const;

	SceneAllocator<U8> getSceneFrameAllocator() const;

	SceneGraph& getSceneGraph()
	{
		return *scene;
	}
	const SceneGraph& getSceneGraph() const
	{
		return *scene;
	}

	Bool isMarkedForDeletion() const
	{
		return (flags & MARKED_FOR_DELETION) != 0;
	}
	void markForDeletion();

	/// Visit this and the children of a specific SceneObject type
	template<typename ScObj, typename Func>
	void visitThisAndChildren(Func func)
	{
		Base::visitThisAndChildren([&](SceneObject& so)
		{
			const Type type = ScObj::getClassType();

			if(so.getType() == type)
			{
				func(so.downCast<ScObj>());
			}
		});
	}

	/// Visit this and the children of a specific SceneObject type
	template<typename ScObj, typename Func>
	void visitThisAndChildren(Func func) const
	{
		Base::visitThisAndChildren([&](const SceneObject& so)
		{
			const Type type = ScObj::getClassType();

			if(so.getType() == type)
			{
				func(so.downCast<ScObj>());
			}
		});
	}

	/// Downcast the class
	template<typename ScObj>
	ScObj& downCast()
	{
		ANKI_ASSERT(ScObj::getClassType() == getType());
		ScObj* out = staticCastPtr<ScObj*>(this);
		return *out;
	}

	/// Downcast the class
	template<typename ScObj>
	const ScObj& downCast() const
	{
		ANKI_ASSERT(ScObj::getClassType() == getType());
		const ScObj* out = staticCastPtr<const ScObj*>(this);
		return *out;
	}

public:
	enum
	{
		MARKED_FOR_DELETION = 1 << 1
	};

	SceneGraph* scene;
	U8 flags; ///< Contains the type and the marked for deletion
};

inline void SceneObjectCallbackCollection::onChildRemoved(
	SceneObject* child, SceneObject* parent)
{
	child->markForDeletion();
}

/// @}

} // end namespace anki

#endif
