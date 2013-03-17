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

/// A hierarchical object
template<typename T, typename Alloc = Allocator<T>>
class Object: public NonCopyable
{
public:
	typedef T Value;
	typedef Vector<Value*, Alloc> Container;

	/// Calls addChild if parent is not nullptr
	Object(Value* parent_, const Alloc& alloc = Alloc())
		: parent(nullptr), childs(alloc)
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
		for(Value* child : childs)
		{
			child->parent = nullptr;
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
		return childs.begin();
	}
	typename Container::iterator getChildrenBegin()
	{
		return childs.begin();
	}
	typename Container::const_iterator getChildrenEnd() const
	{
		return childs.end();
	}
	typename Container::iterator getChildrenEnd()
	{
		return childs.end();
	}

	PtrSize getChildrenSize() const
	{
		return childs.size();
	}
	/// @}

	/// Add a new child
	void addChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child != getSelf() && "Cannot put itself");
		ANKI_ASSERT(child->parent == nullptr && "Child already has parent");
		ANKI_ASSERT(child->findChild(getSelf()) == child->childs.end() 
			&& "Cyclic add");
		ANKI_ASSERT(findChild(child) == childs.end() && "Already a child");

		child->parent = getSelf();
		childs.push_back(child);
	}

	/// Remove a child
	void removeChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child->parent == getSelf() && "Child has other parent");

		typename Container::iterator it = findChild(child);

		ANKI_ASSERT(it != childs.end() && "Child not found");

		childs.erase(it);
		child->parent = nullptr;
	}

	/// Visit
	template<typename Visitor>
	void visitTreeDepth(Visitor& vis)
	{
		vis(*getSelf());

		for(Value* c : childs)
		{
			c->visitTreeDepth(vis);
		}
	}

private:
	Value* parent; ///< May be nullptr
	Container childs;

	/// Cast the Object to the given type
	Value* getSelf()
	{
		return static_cast<Value*>(this);
	}

	/// Find the child
	typename Container::iterator findChild(Value* child)
	{
		typename Container::iterator it =
			std::find(childs.begin(), childs.end(), child);
		return it;
	}
};

/// XXX
template<typename T, typename Alloc = Allocator<T>>
class CleanupObject: public Object<T, Alloc>
{
public:
	typedef T Value;
	typedef Object<Value, Alloc> Base;

	/// @see Object::Object
	CleanupObject(Value* parent_, const Alloc& alloc = Alloc())
		: Base(parent_, alloc)
	{}

	virtual ~CleanupObject()
	{
		if(Base::parent != nullptr)
		{
			Base::parent->removeChild(Base::getSelf());
			Base::parent = nullptr;
		}

		Alloc alloc = Base::childs.get_allocator();

		// Delete all children
		for(Value* child : Base::childs)
		{
			// Set parent to null to prevent the child from removing itself
			child->parent = nullptr;

			// Destroy
			alloc.destroy(child);
			alloc.deallocate(child, 1);
		}

		Base::childs.clear();
	}
};

/// @}
/// @}

} // end namespace anki

#endif
