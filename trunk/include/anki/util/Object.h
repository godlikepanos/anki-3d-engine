#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include "anki/util/Vector.h"
#include "anki/util/StdTypes.h"
#include <algorithm>

namespace anki {

/// @addtogroup util
/// @{
/// @addtogroup patterns
/// @{

/// A hierarchical object
template<typename T>
class Object
{
public:
	typedef T Value;
	typedef Vector<Value*> Container;

	/// Calls addChild if parent is not nullptr
	Object(Value* self_, Value* parent_)
		: self(self_)
	{
		ANKI_ASSERT(self != nullptr && "Self can't be nullptr");
		ANKI_ASSERT(parent_ != this && "Cannot put itself");
		if(parent_ != nullptr)
		{
			parent_->addChild(self);
		}
		parent = parent_;
	}

	/// Delete children from the last entered to the first and update parent
	virtual ~Object()
	{
		if(parent != nullptr)
		{
			parent->removeChild(self);
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
		ANKI_ASSERT(child->parent ==  nullptr && "Child already has parent");

		child->parent = self;
		childs.push_back(child);
	}

	/// Remove a child
	void removeChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child->parent == self);

		typename Container::iterator it =
			std::find(childs.begin(), childs.end(), child);

		ANKI_ASSERT(it != childs.end() && "Child not found");

		childs.erase(it);
		child->parent = nullptr;
	}

private:
	Value* self = nullptr;
	Value* parent = nullptr; ///< May be nullptr
	Container childs;
};
/// @}
/// @}

} // end namespace anki

#endif
