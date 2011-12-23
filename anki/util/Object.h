#ifndef ANKI_UTIL_OBJECT_H
#define ANKI_UTIL_OBJECT_H

#include "anki/util/Assert.h"
#include <vector>
#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>


namespace anki {


/// A hierarchical object
template<typename T>
class Object
{
public:
	typedef T Value;
	typedef std::vector<Object*> Container;
	typedef boost::iterator_range<typename Container::const_iterator>
		ConstIteratorRange;
	typedef boost::iterator_range<typename Container::iterator>
		MutableIteratorRange;

	/// Calls addChild if parent is not NULL
	/// @exception Exception
	Object(Object* parent_)
		: parent(NULL)
	{
		if(parent_ != NULL)
		{
			parent_->addChild(this);
		}
	}

	/// Delete childs from the last entered to the first and update parent
	virtual ~Object()
	{
		if(parent != NULL)
		{
			parent->removeChild(this);
		}

		// Remove all children
		BOOST_FOREACH(Object* child, childs)
		{
			child->parent = NULL;
		}
	}

	/// @name Accessors
	/// @{
	const Object* getParent() const
	{
		return parent;
	}
	Object* getParent()
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
	void addChild(Object* child)
	{
		ANKI_ASSERT(child != NULL && "Null arg");
		ANKI_ASSERT(child->parent ==  NULL && "Child already has parent");

		child->parent = this;
		childs.push_back(child);
	}

	/// Remove a child
	void removeChild(Object* child)
	{
		ANKI_ASSERT(child != NULL && "Null arg");

		typename Container::iterator it =
			std::find(childs.begin(), childs.end(), child);

		ANKI_ASSERT(it != childs.end() && "Child not found");

		childs.erase(it);
		child->parent = NULL;
	}

private:
	Object* parent; ///< May be nullptr
	Container childs;
};


} // end namespace


#endif
