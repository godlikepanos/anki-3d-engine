// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/components/SceneComponent.h>
#include <anki/util/Hierarchy.h>
#include <anki/util/BitMask.h>
#include <anki/util/BitSet.h>
#include <anki/util/List.h>
#include <anki/util/Enum.h>

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

	/// A dummy init for those scene nodes that don't need it.
	ANKI_USE_RESULT Error init()
	{
		return Error::NONE;
	}

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
			err = func(*c, it->isFeedbackComponent());
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
			err = func(*c, it->isFeedbackComponent());
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
			if(it->getComponentClassId() == TComponent::getStaticClassId())
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
			if(it->getComponentClassId() == TComponent::getStaticClassId())
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
			if(el.getComponentClassId() == TComponent::getStaticClassId())
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
			if(el.getComponentClassId() == TComponent::getStaticClassId() && inth-- == 0)
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

	template<typename TComponent>
	const TComponent& getNthComponentOfType(U32 nth) const
	{
		const TComponent* out = tryGetNthComponentOfType<TComponent>(nth);
		ANKI_ASSERT(out);
		return *out;
	}

	template<typename TComponent>
	TComponent& getNthComponentOfType(U32 nth)
	{
		TComponent* out = tryGetNthComponentOfType<TComponent>(nth);
		ANKI_ASSERT(out);
		return *out;
	}

	/// Get the nth component.
	template<typename TComponent>
	TComponent& getComponentAt(U32 idx)
	{
		ANKI_ASSERT(m_components[idx].getComponentClassId() == TComponent::getStaticClassId());
		SceneComponent* c = m_components[idx];
		return *static_cast<TComponent*>(c);
	}

	/// Get the nth component.
	template<typename TComponent>
	const TComponent& getComponentAt(U32 idx) const
	{
		ANKI_ASSERT(m_components[idx].getComponentClassId() == TComponent::getStaticClassId());
		const SceneComponent* c = m_components[idx];
		return *static_cast<const TComponent*>(c);
	}

	U32 getComponentCount() const
	{
		return m_components.getSize();
	}

	template<typename TComponent>
	U32 countComponentsOfType() const
	{
		U32 count = 0;
		const Error err = iterateComponentsOfType<TComponent>([&](const TComponent& c) -> Error {
			++count;
			return Error::NONE;
		});
		(void)err;
		return count;
	}

protected:
	/// Create and append a component to the components container. The SceneNode has the ownership.
	template<typename TComponent>
	TComponent* newComponent()
	{
		TComponent* comp = getAllocator().newInstance<TComponent>(this);
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
		/// Encodes the SceneComponent's class ID, the SceneComponent* and if it's feedback component or not.
		PtrSize m_combo;

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

		U8 getComponentClassId() const
		{
			return m_combo & 0xFF;
		}

		Bool isFeedbackComponent() const
		{
			return m_combo & PtrSize(1 << 7);
		}

	private:
		void set(SceneComponent* comp)
		{
			m_combo = ptrToNumber(comp) << 8;
			m_combo |= PtrSize(comp->isFeedbackComponent()) << 7;
			m_combo |= PtrSize(comp->getClassId()) & 0x7F;
			ANKI_ASSERT(getPtr() == comp);
			ANKI_ASSERT(getComponentClassId() == comp->getClassId());
			ANKI_ASSERT(isFeedbackComponent() == comp->isFeedbackComponent());
		}

		SceneComponent* getPtr() const
		{
			return numberToPtr<SceneComponent*>(m_combo >> 8);
		}
	};

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
