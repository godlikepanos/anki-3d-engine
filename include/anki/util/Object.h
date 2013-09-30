#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include "anki/util/Allocator.h"
#include "anki/util/NonCopyable.h"
#include <algorithm>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup patterns
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
template<typename T, typename Alloc = Allocator<T>,
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
	/// @param callbacks_ A set of callbacks
	Object(
		Value* parent_, 
		const Alloc& alloc = Alloc(),
		const CallbackCollection& callbacks_ = CallbackCollection())
		:	parent(nullptr), // Set to nullptr or prepare for assertion
			children(alloc), 
			callbacks(callbacks_)
	{
		if(parent_ != nullptr)
		{
			parent_->addChild(getSelf());
		}
	}

	/// Delete children from the last entered to the first and update parent
	virtual ~Object()
	{
		if(parent != nullptr)
		{
			parent->removeChild(getSelf());
		}

		// Remove all children (fast version)
		for(Value* child : children)
		{
			child->parent = nullptr;

			// Pass the parent as nullptr because at this point there is 
			// nothing you should do with a deleteding parent
			callbacks.onChildRemoved(child, nullptr);
		}
	}

	/// @name Accessors
	/// @{
	const Value* getParent() const
	{
		return parent;
	}
	Value* getParent()
	{
		return parent;
	}

	PtrSize getChildrenSize() const
	{
		return children.size();
	}

	Value& getChild(PtrSize i)
	{
		ANKI_ASSERT(i < children.size());
		return *children[i];
	}
	const Value& getChild(PtrSize i) const
	{
		ANKI_ASSERT(i < children.size());
		return *children[i];
	}

	Alloc getAllocator() const
	{
		return children.get_allocator();
	}
	/// @}

	/// Add a new child
	void addChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child != getSelf() && "Cannot put itself");
		ANKI_ASSERT(child->parent == nullptr && "Child already has parent");
		ANKI_ASSERT(child->findChild(getSelf()) == child->children.end()
			&& "Cyclic add");
		ANKI_ASSERT(findChild(child) == children.end() && "Already a child");

		child->parent = getSelf();
		children.push_back(child);

		callbacks.onChildAdded(child, getSelf());
	}

	/// Remove a child
	void removeChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child->parent == getSelf() && "Child has other parent");

		typename Container::iterator it = findChild(child);

		ANKI_ASSERT(it != children.end() && "Child not found");

		children.erase(it);
		child->parent = nullptr;

		callbacks.onChildRemoved(child, getSelf());
	}

	/// Visit the children and the children's children. Use it with lambda
	template<typename VisitorFunc>
	void visitChildren(VisitorFunc vis)
	{
		for(Value* c : children)
		{
			vis(*c);
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
		while(root->parent != nullptr)
		{
			root = root->parent;
		}

		root->visitThisAndChildren(vis);
	}

private:
	Value* parent; ///< May be nullptr
	Container children;
	CallbackCollection callbacks; /// A set of callbacks

	/// Cast the Object to the given type
	Value* getSelf()
	{
		return static_cast<Value*>(this);
	}

	/// Find the child
	typename Container::iterator findChild(Value* child)
	{
		typename Container::iterator it =
			std::find(children.begin(), children.end(), child);
		return it;
	}
};
/// @}
/// @}

} // end namespace anki

#endif
