#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include "anki/util/Vector.h"
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
template<typename T, typename Alloc = HeapAllocator<T>,
	typename TCallbackCollection = ObjectCallbackCollection<T>>
class Object: public NonCopyable
{
public:
	typedef T Value;
	typedef Vector<Value*, Alloc> Container;
	typedef TCallbackCollection CallbackCollection;

	/// Calls addChild if parent is not nullptr
	///
	/// @param parent_ The parent. Can be nullptr
	/// @param alloc The allocator to use on internal allocations
	/// @param callbacks A set of callbacks
	Object(
		Value* parent, 
		const Alloc& alloc = Alloc(),
		const CallbackCollection& callbacks = CallbackCollection())
		:	m_parent(nullptr), // Set to nullptr or prepare for assertion
			m_children(alloc), 
			m_callbacks(callbacks)
	{
		if(parent != nullptr)
		{
			parent->addChild(getSelf());
		}
	}

	/// Delete children from the last entered to the first and update parent
	virtual ~Object()
	{
		if(m_parent != nullptr)
		{
			m_parent->removeChild(getSelf());
		}

		// Remove all children (fast version)
		for(Value* child : m_children)
		{
			child->m_parent = nullptr;

			// Pass the parent as nullptr because at this point there is 
			// nothing you should do with a deleteding parent
			m_callbacks.onChildRemoved(child, nullptr);
		}
	}

	/// @name Accessors
	/// @{
	const Value* getParent() const
	{
		return m_parent;
	}
	Value* getParent()
	{
		return m_parent;
	}

	PtrSize getChildrenSize() const
	{
		return m_children.size();
	}

	Value& getChild(PtrSize i)
	{
		ANKI_ASSERT(i < m_children.size());
		return *m_children[i];
	}
	const Value& getChild(PtrSize i) const
	{
		ANKI_ASSERT(i < m_children.size());
		return *m_children[i];
	}

	Alloc getAllocator() const
	{
		return m_children.get_allocator();
	}
	/// @}

	/// Add a new child
	void addChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child != getSelf() && "Cannot put itself");
		ANKI_ASSERT(child->m_parent == nullptr && "Child already has parent");
		ANKI_ASSERT(child->findChild(getSelf()) == child->m_children.end()
			&& "Cyclic add");
		ANKI_ASSERT(findChild(child) == m_children.end() && "Already a child");

		child->m_parent = getSelf();
		m_children.push_back(child);

		m_callbacks.onChildAdded(child, getSelf());
	}

	/// Remove a child
	void removeChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child->m_parent == getSelf() && "Child has other parent");

		typename Container::iterator it = findChild(child);

		ANKI_ASSERT(it != m_children.end() && "Child not found");

		m_children.erase(it);
		child->m_parent = nullptr;

		m_callbacks.onChildRemoved(child, getSelf());
	}

	/// Visit the children and the children's children. Use it with lambda
	template<typename VisitorFunc>
	void visitChildren(VisitorFunc vis)
	{
		for(Value* c : m_children)
		{
			vis(*c);
			c->visitChildren(vis);
		}
	}

	/// Visit this object and move to the children. Use it with lambda
	template<typename VisitorFunc>
	void visitThisAndChildren(VisitorFunc vis)
	{
		vis(*getSelf());

		visitChildren(vis);
	}

	/// Visit the whole tree. Use it with lambda
	template<typename VisitorFunc>
	void visitTree(VisitorFunc vis)
	{
		// Move to root
		Value* root = getSelf();
		while(root->m_parent != nullptr)
		{
			root = root->m_parent;
		}

		root->visitThisAndChildren(vis);
	}

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
	typename Container::iterator findChild(Value* child)
	{
		typename Container::iterator it =
			std::find(m_children.begin(), m_children.end(), child);
		return it;
	}
};
/// @}

} // end namespace anki

#endif
