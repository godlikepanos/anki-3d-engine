#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include <vector>
#include <boost/range/iterator_range.hpp>

namespace anki {

/// @addtogroup util
/// @{

/// A hierarchical object
template<typename T>
class Object
{
public:
	typedef T Value;
	typedef std::vector<Value*> Container;
	typedef boost::iterator_range<typename Container::const_iterator>
		ConstIteratorRange;
	typedef boost::iterator_range<typename Container::iterator>
		MutableIteratorRange;

	/// Calls addChild if parent is not nullptr
	Object(Value* self_, Value* parent_)
		: self(self_)
	{
		ANKI_ASSERT(self != nullptr && "Self can't be nullptr");
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

	ConstIteratorRange getChildren() const
	{
		return ConstIteratorRange(childs.begin(), childs.end());
	}
	MutableIteratorRange getChildren()
	{
		return MutableIteratorRange(childs.begin(), childs.end());
	}

	size_t getChildrenSize() const
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

} // end namespace anki

#endif
