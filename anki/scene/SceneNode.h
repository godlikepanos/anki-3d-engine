// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/util/Hierarchy.h>
#include <anki/util/BitMask.h>
#include <anki/util/BitSet.h>
#include <anki/util/List.h>
#include <anki/util/Enum.h>
#include <anki/scene/components/SceneComponent.h>

namespace anki
{

// Forward
class ResourceManager;

/// @addtogroup scene
/// @{

/// Interface class backbone of scene
class SceneNode : public Hierarchy<SceneNode>, public IntrusiveListEnabled<SceneNode>
{
public:
	using Base = Hierarchy<SceneNode>;

	/// The one and only constructor.
	/// @param scene The owner scene.
	/// @param name The unique name of the node. If it's empty the the node is not searchable.
	SceneNode(SceneGraph* scene, CString name);

	/// Unregister node
	virtual ~SceneNode();

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	/// Return the name. It may be empty for nodes that we don't want to track
	CString getName() const
	{
		return (!m_name.isEmpty()) ? m_name.toCString() : CString();
	}

	U64 getUuid() const
	{
		return m_uuid;
	}

	Bool getMarkedForDeletion() const
	{
		return m_markedForDeletion;
	}

	void setMarkedForDeletion();

	Timestamp getGlobalTimestamp() const;

	Timestamp getComponentMaxTimestamp() const
	{
		return m_maxComponentTimestamp;
	}

	void setComponentMaxTimestamp(Timestamp maxComponentTimestamp)
	{
		ANKI_ASSERT(maxComponentTimestamp > 0);
		m_maxComponentTimestamp = maxComponentTimestamp;
	}

	SceneAllocator<U8> getAllocator() const;

	SceneFrameAllocator<U8> getFrameAllocator() const;

	void addChild(SceneNode* obj)
	{
		Base::addChild(getAllocator(), obj);
	}

	/// This is called by the scene every frame after logic and before rendering. By default it does nothing.
	/// @param prevUpdateTime Timestamp of the previous update
	/// @param crntTime Timestamp of this update
	virtual ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		return Error::NONE;
	}

	/// Iterate all components.
	template<typename TFunct>
	ANKI_USE_RESULT Error iterateComponents(TFunct func) const
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			const SceneComponent* c = *it;
			err = func(*c);
		}

		return err;
	}

	/// Iterate all components.
	template<typename TFunct>
	ANKI_USE_RESULT Error iterateComponents(TFunct func)
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			SceneComponent* c = *it;
			err = func(*c);
		}

		return err;
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	ANKI_USE_RESULT Error iterateComponentsOfType(TFunct func) const
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			if(it->getComponentType() == TComponent::CLASS_TYPE)
			{
				const SceneComponent* comp = *it;
				err = func(static_cast<const TComponent&>(*comp));
			}
		}

		return err;
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	ANKI_USE_RESULT Error iterateComponentsOfType(TFunct func)
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			if(it->getComponentType() == TComponent::CLASS_TYPE)
			{
				SceneComponent* comp = *it;
				err = func(static_cast<TComponent&>(*comp));
			}
		}

		return err;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent* tryGetFirstComponentOfType() const
	{
		for(const ComponentsArrayElement& el : m_components)
		{
			if(el.getComponentType() == TComponent::CLASS_TYPE)
			{
				const SceneComponent* comp = el;
				return static_cast<const TComponent*>(comp);
			}
		}
		return nullptr;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	TComponent* tryGetFirstComponentOfType()
	{
		const TComponent* c = static_cast<const SceneNode*>(this)->tryGetFirstComponentOfType<TComponent>();
		return const_cast<TComponent*>(c);
	}

	/// Get a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent& getFirstComponentOfType() const
	{
		const TComponent* out = tryGetFirstComponentOfType<TComponent>();
		ANKI_ASSERT(out != nullptr);
		return *out;
	}

	/// Get a pointer to the first component of the requested type
	template<typename TComponent>
	TComponent& getFirstComponentOfType()
	{
		const TComponent& c = static_cast<const SceneNode*>(this)->getFirstComponentOfType<TComponent>();
		return const_cast<TComponent&>(c);
	}

	/// Try geting a pointer to the nth component of the requested type.
	template<typename TComponent>
	const TComponent* tryGetNthComponentOfType(U32 nth) const
	{
		I32 inth = I32(nth);
		for(const ComponentsArrayElement& el : m_components)
		{
			if(el.getComponentType() == TComponent::CLASS_TYPE && inth-- == 0)
			{
				const SceneComponent* comp = el;
				return static_cast<const TComponent*>(comp);
			}
		}
		return nullptr;
	}

	/// Try geting a pointer to the nth component of the requested type.
	template<typename TComponent>
	TComponent* tryGetNthComponentOfType(U32 nth)
	{
		const TComponent* c = static_cast<const SceneNode*>(this)->tryGetNthComponentOfType<TComponent>(nth);
		return const_cast<TComponent*>(c);
	}

	/// Get the nth component.
	template<typename TComponent>
	TComponent& getComponentAt(U32 idx)
	{
		ANKI_ASSERT(m_components[idx].getComponentType() == TComponent::CLASS_TYPE);
		SceneComponent* c = m_components[idx];
		return *static_cast<TComponent*>(c);
	}

	/// Get the nth component.
	template<typename TComponent>
	const TComponent& getComponentAt(U32 idx) const
	{
		ANKI_ASSERT(m_components[idx].getComponentType() == TComponent::CLASS_TYPE);
		const SceneComponent* c = m_components[idx];
		return *static_cast<const TComponent*>(c);
	}

	U32 getComponentCount() const
	{
		return m_components.getSize();
	}

protected:
	/// Create and append a component to the components container. The SceneNode has the ownership.
	template<typename TComponent, typename... TArgs>
	TComponent* newComponent(TArgs&&... args)
	{
		TComponent* comp = getAllocator().newInstance<TComponent>(std::forward<TArgs>(args)...);
		m_components.emplaceBack(getAllocator(), comp);
		return comp;
	}

	ResourceManager& getResourceManager();

private:
	/// This class packs a pointer to a SceneComponent and its type at the same 64bit value. Used to avoid cache misses
	/// when iterating the m_components.
	class ComponentsArrayElement
	{
	public:
		PtrSize m_combo; ///< Encodes the SceneComponentType in the high 8bits and the SceneComponent* to the remaining.

		ComponentsArrayElement(SceneComponent* comp)
		{
			set(comp);
		}

		ComponentsArrayElement(const ComponentsArrayElement& b)
			: m_combo(b.m_combo)
		{
		}

		ComponentsArrayElement& operator=(const ComponentsArrayElement& b)
		{
			m_combo = b.m_combo;
			return *this;
		}

		operator SceneComponent*()
		{
			return getPtr();
		}

		operator const SceneComponent*() const
		{
			return getPtr();
		}

		const SceneComponent* operator->() const
		{
			return getPtr();
		}

		SceneComponent* operator->()
		{
			return getPtr();
		}

		SceneComponentType getComponentType() const
		{
			return SceneComponentType(m_combo & 0xFF);
		}

	private:
		void set(SceneComponent* comp)
		{
			m_combo = ptrToNumber(comp) << 8;
			m_combo |= PtrSize(comp->getType());
			ANKI_ASSERT(getPtr() == comp);
		}

		SceneComponent* getPtr() const
		{
			return numberToPtr<SceneComponent*>(m_combo >> 8);
		}
	};

	static_assert(sizeof(SceneComponentType) == 1, "Wrong size");
	static_assert(sizeof(ComponentsArrayElement) == sizeof(void*), "Wrong size");

	SceneGraph* m_scene = nullptr;
	U64 m_uuid;
	String m_name; ///< A unique name.

	DynamicArray<ComponentsArrayElement> m_components;

	Timestamp m_maxComponentTimestamp = 0;

	Bool m_markedForDeletion = false;
};
/// @}

} // end namespace anki
