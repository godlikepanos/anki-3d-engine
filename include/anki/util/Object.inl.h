// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
void Object<T, TAlloc, TCallbackCollection>::destroy(Allocator alloc)
{
	if(m_parent != nullptr)
	{
		m_parent->removeChild(alloc, getSelf());
		m_parent = nullptr;
	}

	// Remove all children (fast version)
	for(typename Container::Iterator it : m_children)
	{
		Value* child = *it;
		child->m_parent = nullptr;

		// Pass the parent as nullptr because at this point there is 
		// nothing you should do with a deleteding parent
		m_callbacks.onChildRemoved(child, nullptr);
	}

	m_children.destroy(alloc);
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
Error Object<T, TAlloc, TCallbackCollection>::addChild(
	Allocator alloc, Value* child)
{
	ANKI_ASSERT(child != nullptr && "Null arg");
	ANKI_ASSERT(child != getSelf() && "Cannot put itself");
	ANKI_ASSERT(child->m_parent == nullptr && "Child already has parent");
	ANKI_ASSERT(child->findChild(getSelf()) == child->m_children.getEnd()
		&& "Cyclic add");
	ANKI_ASSERT(findChild(child) == m_children.getEnd() 
		&& "Already has that child");

	child->m_parent = getSelf();
	Error err = m_children.emplaceBack(alloc, child);

	if(!err)
	{
		m_callbacks.onChildAdded(child, getSelf());
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
void Object<T, TAlloc, TCallbackCollection>::removeChild(
	Allocator alloc, Value* child)
{
	ANKI_ASSERT(child != nullptr && "Null arg");
	ANKI_ASSERT(child->m_parent == getSelf() && "Child has other parent");

	typename Container::Iterator it = findChild(child);

	ANKI_ASSERT(it != m_children.getEnd() && "Child not found");

	m_children.erase(it);
	child->m_parent = nullptr;

	m_callbacks.onChildRemoved(child, getSelf());
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
template<typename VisitorFunc>
Error Object<T, TAlloc, TCallbackCollection>::visitChildren(VisitorFunc vis)
{
	Error err = ErrorCode::NONE;
	typename Container::Iterator it = m_children.getBegin();
	for(; it != m_children.getEnd() && !err; it++)
	{
		err = vis(*(*it));

		if(!err)
		{
			err = (*it)->visitChildren(vis);
		}
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
template<typename VisitorFunc>
Error Object<T, TAlloc, TCallbackCollection>::visitThisAndChildren(
	VisitorFunc vis)
{
	Error err = vis(*getSelf());

	if(!err)
	{
		err = visitChildren(vis);
	}

	return err;
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
template<typename VisitorFunc>
Error Object<T, TAlloc, TCallbackCollection>::visitTree(VisitorFunc vis)
{
	// Move to root
	Value* root = getSelf();
	while(root->m_parent != nullptr)
	{
		root = root->m_parent;
	}

	return root->visitThisAndChildren(vis);
}

//==============================================================================
template<typename T, typename TAlloc, typename TCallbackCollection>
template<typename VisitorFunc>
Error Object<T, TAlloc, TCallbackCollection>::visitChildrenMaxDepth(
	I maxDepth, VisitorFunc vis)
{
	ANKI_ASSERT(maxDepth >= 0);
	Error err = ErrorCode::NONE;
	--maxDepth;

	typename Container::Iterator it = m_children.getBegin();
	for(; it != m_children.getEnd() && !err; ++it)
	{
		err = vis(*(*it));

		if(!err && maxDepth >= 0)
		{
			err = (*it)->visitChildrenMaxDepth(maxDepth, vis);
		}
	}

	return err;
}

} // end namespace anki

