// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
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

	/// Iterate all components
	template<typename Func>
	ANKI_USE_RESULT Error iterateComponents(Func func) const
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			err = func(*(*it));
		}

		return err;
	}

	/// Iterate all components of a specific type
	template<typename Component, typename Func>
	ANKI_USE_RESULT Error iterateComponentsOfType(Func func) const
	{
		Error err = Error::NONE;
		auto it = m_components.getBegin();
		auto end = m_components.getEnd();
		for(; !err && it != end; ++it)
		{
			auto* comp = *it;
			if(comp->getType() == Component::CLASS_TYPE)
			{
				err = func(*static_cast<Component*>(comp));
			}
		}

		return err;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	Component* tryGetComponent()
	{
		U count = m_components.getSize();
		while(count-- != 0)
		{
			SceneComponent* comp = m_components[count];
			if(comp->getType() == Component::CLASS_TYPE)
			{
				return static_cast<Component*>(comp);
			}
		}
		return nullptr;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	const Component* tryGetComponent() const
	{
		U count = m_components.getSize();
		while(count-- != 0)
		{
			const SceneComponent* comp = m_components[count];
			if(comp->getType() == Component::CLASS_TYPE)
			{
				return static_cast<const Component*>(comp);
			}
		}
		return nullptr;
	}

	/// Get a pointer to the first component of the requested type
	template<typename Component>
	Component& getComponent()
	{
		Component* out = tryGetComponent<Component>();
		ANKI_ASSERT(out != nullptr);
		return *out;
	}

	/// Get a pointer to the first component of the requested type
	template<typename Component>
	const Component& getComponent() const
	{
		const Component* out = tryGetComponent<Component>();
		ANKI_ASSERT(out != nullptr);
		return *out;
	}

	/// Get the nth component.
	template<typename Component>
	Component& getComponentAt(U idx)
	{
		ANKI_ASSERT(m_components[idx]->getType() == Component::CLASS_TYPE);
		return *static_cast<Component*>(m_components[idx]);
	}

	/// Get the nth component.
	template<typename Component>
	const Component& getComponentAt(U idx) const
	{
		ANKI_ASSERT(m_components[idx]->getType() == Component::CLASS_TYPE);
		return *static_cast<const Component*>(m_components[idx]);
	}

	U getComponentCount() const
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
	SceneGraph* m_scene = nullptr;
	U64 m_uuid;
	String m_name; ///< A unique name

	DynamicArray<SceneComponent*> m_components;

	Timestamp m_maxComponentTimestamp = 0;

	Bool8 m_markedForDeletion = false;
};
/// @}

} // end namespace anki
