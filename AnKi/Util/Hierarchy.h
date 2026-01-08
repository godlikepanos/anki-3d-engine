// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Util/Assert.h>
#include <AnKi/Util/StdTypes.h>

namespace anki {

// In can be used to build a hierarchical object. Doesn't have any data and the typename T needs to have a specific interface.
// It needs to implement two methods: getHierarchyParent() and getHierarchyChildren() like this:
// Container<T*>& getHierarchyChildren()
// const Container<T*>& getHierarchyChildren() const
// T* getHierarchyParent() const
// T*& getHierarchyParent()
template<typename T>
class IntrusiveHierarchy
{
public:
	using Value = T;

	IntrusiveHierarchy() = default;

	IntrusiveHierarchy(const IntrusiveHierarchy&) = delete; // Non-copyable

	IntrusiveHierarchy& operator=(const IntrusiveHierarchy&) = delete; // Non-copyable

	const Value* getParent() const
	{
		return parent();
	}

	Value* getParent()
	{
		return parent();
	}

	auto& getChildren()
	{
		return children();
	}

	const auto& getChildren() const
	{
		return children();
	}

	Value& getChild(U32 i)
	{
		return *(*(children().getBegin() + i));
	}

	const Value& getChild(U32 i) const
	{
		return *(*(children().getBegin() + i));
	}

	Bool hasChildren() const
	{
		return !children().isEmpty();
	}

	// Add a new child.
	void addChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child != self() && "Cannot put itself");
		ANKI_ASSERT(child->parent() == nullptr && "Child already has parent");
		ANKI_ASSERT(child->findChild(self()) == child->children().getEnd() && "Cyclic add");
		ANKI_ASSERT(findChild(child) == children().getEnd() && "Already has that child");

		child->parent() = self();
		children().emplaceBack(child);
	}

	// Remove a child.
	void removeChild(Value* child)
	{
		ANKI_ASSERT(child != nullptr && "Null arg");
		ANKI_ASSERT(child->parent() == self() && "Child has other parent");

		auto it = findChild(child);

		ANKI_ASSERT(it != children().getEnd() && "Child not found");

		children().erase(it);
		child->parent() = nullptr;
	}

	void setParent(Value* parent)
	{
		ANKI_ASSERT(parent);
		static_cast<IntrusiveHierarchy*>(parent)->addChild(self());
	}

	void removeParent()
	{
		if(parent())
		{
			static_cast<IntrusiveHierarchy*>(parent())->removeChild(self());
		}
	}

	// Visit the children and the children's children. Use it with lambda
	template<typename TVisitorFunc>
	FunctorContinue visitChildren(TVisitorFunc vis)
	{
		auto it = children().getBegin();
		FunctorContinue cont = FunctorContinue::kContinue;
		for(; it != children().getEnd() && cont == FunctorContinue::kContinue; it++)
		{
			cont = vis(*(*it));

			if(cont == FunctorContinue::kContinue)
			{
				cont = (*it)->visitChildren(vis);
			}
		}

		return cont;
	}

	// Visit this object and move to the children. Use it with lambda
	template<typename TVisitorFunc>
	FunctorContinue visitThisAndChildren(TVisitorFunc vis)
	{
		FunctorContinue cont = vis(*self());
		if(cont == FunctorContinue::kContinue)
		{
			cont = visitChildren(vis);
		}
		return cont;
	}

	// Visit the whole tree. Use it with lambda
	template<typename TVisitorFunc>
	void visitTree(TVisitorFunc vis)
	{
		// Move to root
		Value* root = self();
		while(root->parent() != nullptr)
		{
			root = root->parent();
		}

		root->visitThisAndChildren(vis);
	}

	// Visit the children and limit the depth. Use it with lambda.
	template<typename TVisitorFunc>
	FunctorContinue visitChildrenMaxDepth(I maxDepth, TVisitorFunc vis)
	{
		ANKI_ASSERT(maxDepth >= 0);
		--maxDepth;

		FunctorContinue cont = FunctorContinue::kContinue;
		auto it = children().getBegin();
		for(; it != children().getEnd() && cont == FunctorContinue::kContinue; ++it)
		{
			cont = vis(*(*it));

			if(cont == FunctorContinue::kContinue && maxDepth >= 0)
			{
				cont = (*it)->visitChildrenMaxDepth(maxDepth, vis);
			}
		}

		return cont;
	}

private:
	Value* self()
	{
		return static_cast<Value*>(this);
	}

	const Value* self() const
	{
		return static_cast<const Value*>(this);
	}

	auto& children()
	{
		return self()->getHierarchyChildren();
	}

	const auto& children() const
	{
		return self()->getHierarchyChildren();
	}

	Value*& parent()
	{
		return self()->getHierarchyParent();
	}

	const Value* parent() const
	{
		return self()->getParent();
	}

	// Find the child given its pointer
	auto findChild(Value* child)
	{
		return children().find(child);
	}
};

} // end namespace anki
