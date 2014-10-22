// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include "anki/util/List.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"
#include <algorithm>

namespace anki {

/// @addtogroup util_patterns
/// @{

/// The default set of callbacks. They do nothing
template<typename T>
struct ObjectCallbackCollection
{
	/// Called when a child is been removed from a parent
	void onChildRemoved(T* child, T* parent)
	{
		ANKI_ASSERT(child);
		// Do nothing
	}

	/// Called when a child is been added to a parent
	void onChildAdded(T* child, T* parent)
	{
		ANKI_ASSERT(child && parent);
		// Do nothing
	}
};

/// A hierarchical object
template<typename T, typename TAlloc = HeapAllocator<T>,
	typename TCallbackCollection = ObjectCallbackCollection<T>>
class Object: public NonCopyable
{
public:
	using Value = T;
	using Allocator = TAlloc;
	using Container = List<Value*, Allocator>;
	using CallbackCollection = TCallbackCollection;

	/// @param callbacks A set of callbacks
	Object(const CallbackCollection& callbacks = CallbackCollection())
	:	m_parent(nullptr),
		m_callbacks(callbacks)
	{}

	/// Delete children from the last entered to the first and update parent
	virtual ~Object()
	{
		ANKI_ASSERT(m_parent == nullptr && m_children.isEmpty()
			&& "Requires manual desruction");
	}

	void destroy(Allocator alloc);

	const Value* getParent() const
	{
		return m_parent;
	}

	Value* getParent()
	{
		return m_parent;
	}

	Value& getChild(PtrSize i)
	{
		return *(*(m_children.getBegin() + i));
	}

	const Value& getChild(PtrSize i) const
	{
		return *(*(m_children.getBegin() + i));
	}

	/// Add a new child.
	ANKI_USE_RESULT Error addChild(Allocator alloc, Value* child);

	/// Remove a child.
	void removeChild(Allocator alloc, Value* child);

	/// Visit the children and the children's children. Use it with lambda
	template<typename VisitorFunc>
	ANKI_USE_RESULT Error visitChildren(VisitorFunc vis);

	/// Visit this object and move to the children. Use it with lambda
	template<typename VisitorFunc>
	ANKI_USE_RESULT Error visitThisAndChildren(VisitorFunc vis);

	/// Visit the whole tree. Use it with lambda
	template<typename VisitorFunc>
	ANKI_USE_RESULT Error visitTree(VisitorFunc vis);

	/// Visit the children and limit the depth. Use it with lambda.
	template<typename VisitorFunc>
	ANKI_USE_RESULT Error visitChildrenMaxDepth(I maxDepth, VisitorFunc vis);

private:
	Value* m_parent; ///< May be nullptr
	Container m_children;
	CallbackCollection m_callbacks; /// A set of callbacks

	/// Cast the Object to the given type
	Value* getSelf()
	{
		return static_cast<Value*>(this);
	}

	/// Find the child
	typename Container::Iterator findChild(Value* child)
	{
		typename Container::Iterator it = m_children.find(child);
		return it;
	}
};
/// @}

} // end namespace anki

#include "anki/util/Object.inl.h"

#endif
