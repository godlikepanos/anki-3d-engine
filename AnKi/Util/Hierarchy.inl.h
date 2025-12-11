// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

namespace anki {

template<typename T, typename TMemoryPool>
void Hierarchy<T, TMemoryPool>::destroy()
{
	if(m_parent != nullptr)
	{
		m_parent->removeChild(getSelf());
		m_parent = nullptr;
	}

	// Remove all children (fast version)
	auto it = m_children.getBegin();
	auto end = m_children.getEnd();
	for(; it != end; ++it)
	{
		Value* child = *it;
		child->m_parent = nullptr;
	}

	m_children.destroy();
}

template<typename T, typename TMemoryPool>
void Hierarchy<T, TMemoryPool>::addChild(Value* child)
{
	ANKI_ASSERT(child != nullptr && "Null arg");
	ANKI_ASSERT(child != getSelf() && "Cannot put itself");
	ANKI_ASSERT(child->m_parent == nullptr && "Child already has parent");
	ANKI_ASSERT(child->findChild(getSelf()) == child->m_children.getEnd() && "Cyclic add");
	ANKI_ASSERT(findChild(child) == m_children.getEnd() && "Already has that child");

	child->m_parent = getSelf();
	m_children.emplaceBack(child);
}

template<typename T, typename TMemoryPool>
void Hierarchy<T, TMemoryPool>::removeChild(Value* child)
{
	ANKI_ASSERT(child != nullptr && "Null arg");
	ANKI_ASSERT(child->m_parent == getSelf() && "Child has other parent");

	typename Container::Iterator it = findChild(child);

	ANKI_ASSERT(it != m_children.getEnd() && "Child not found");

	m_children.erase(it);
	child->m_parent = nullptr;
}

template<typename T, typename TMemoryPool>
template<typename TVisitorFunc>
FunctorContinue Hierarchy<T, TMemoryPool>::visitChildren(TVisitorFunc vis)
{
	auto it = m_children.getBegin();
	FunctorContinue cont = FunctorContinue::kContinue;
	for(; it != m_children.getEnd() && cont == FunctorContinue::kContinue; it++)
	{
		cont = vis(*(*it));

		if(cont == FunctorContinue::kContinue)
		{
			cont = (*it)->visitChildren(vis);
		}
	}

	return cont;
}

template<typename T, typename TMemoryPool>
template<typename TVisitorFunc>
void Hierarchy<T, TMemoryPool>::visitThisAndChildren(TVisitorFunc vis)
{
	const FunctorContinue cont = vis(*getSelf());

	if(cont == FunctorContinue::kContinue)
	{
		visitChildren(vis);
	}
}

template<typename T, typename TMemoryPool>
template<typename TVisitorFunc>
void Hierarchy<T, TMemoryPool>::visitTree(TVisitorFunc vis)
{
	// Move to root
	Value* root = getSelf();
	while(root->m_parent != nullptr)
	{
		root = root->m_parent;
	}

	root->visitThisAndChildren(vis);
}

template<typename T, typename TMemoryPool>
template<typename TVisitorFunc>
FunctorContinue Hierarchy<T, TMemoryPool>::visitChildrenMaxDepth(I maxDepth, TVisitorFunc vis)
{
	ANKI_ASSERT(maxDepth >= 0);
	--maxDepth;

	FunctorContinue cont = FunctorContinue::kContinue;
	auto it = m_children.getBegin();
	for(; it != m_children.getEnd() && cont == FunctorContinue::kContinue; ++it)
	{
		cont = vis(*(*it));

		if(cont == FunctorContinue::kContinue && maxDepth >= 0)
		{
			cont = (*it)->visitChildrenMaxDepth(maxDepth, vis);
		}
	}

	return cont;
}

} // end namespace anki
