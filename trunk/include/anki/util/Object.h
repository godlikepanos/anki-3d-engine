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

/// The default object deleter. It does nothing
template<typename T>
struct ObjectDeleter
{
	void operator()(T*)
	{
		// Do nothing
	}
};

/// A hierarchical object
template<typename T, typename Alloc = Allocator<T>,
	typename Deleter = ObjectDeleter<T>>
class Object: public NonCopyable
{
public:
	typedef T Value;
	typedef Vector<Value*, Alloc> Container;

	/// Calls addChild if parent is not nullptr
	Object(Value* parent_, const Alloc& alloc = Alloc(),
		const Deleter& del = Deleter())
		: parent(nullptr), children(alloc), deleter(del)
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

		Alloc alloc = children.get_allocator();

		// Remove all children (fast version)
		for(Value* child : children)
		{
			child->parent = nullptr;

			deleter(child);
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

	typename Container::const_iterator getChildrenBegin() const
	{
		return children.begin();
	}
	typename Container::iterator getChildrenBegin()
	{
		return children.begin();
	}
	typename Container::const_iterator getChildrenEnd() const
	{
		return children.end();
	}
	typename Container::iterator getChildrenEnd()
	{
		return children.end();
	}

	PtrSize getChildrenSize() const
	{
		return children.size();
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
	}

	/// Visit
	template<typename Visitor>
	void visitTreeDepth(Visitor& vis)
	{
		vis(*getSelf());

		for(Value* c : children)
		{
			c->visitTreeDepth(vis);
		}
	}

private:
	Value* parent; ///< May be nullptr
	Container children;
	Deleter deleter;

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
