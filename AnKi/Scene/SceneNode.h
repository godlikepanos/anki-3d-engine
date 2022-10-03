// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <AnKi/Scene/Components/SceneComponent.h>
#include <AnKi/Util/Hierarchy.h>
#include <AnKi/Util/BitMask.h>
#include <AnKi/Util/BitSet.h>
#include <AnKi/Util/List.h>
#include <AnKi/Util/Enum.h>

namespace anki {

// Forward
class ResourceManager;
class ConfigSet;

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
	Error init()
	{
		return Error::kNone;
	}

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	const SceneGraph& getSceneGraph() const
	{
		return *m_scene;
	}

	const ConfigSet& getConfig() const;

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

	HeapMemoryPool& getMemoryPool() const;

	StackMemoryPool& getFrameMemoryPool() const;

	void addChild(SceneNode* obj)
	{
		Base::addChild(getMemoryPool(), obj);
	}

	/// This is called by the scenegraph every frame after all component updates. By default it does nothing.
	/// @param prevUpdateTime Timestamp of the previous update
	/// @param crntTime Timestamp of this update
	virtual Error frameUpdate([[maybe_unused]] Second prevUpdateTime, [[maybe_unused]] Second crntTime)
	{
		return Error::kNone;
	}

	/// Iterate all components.
	template<typename TFunct>
	void iterateComponents(TFunct func) const
	{
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			// Use the m_componentInfos to check if it's feedback component for possibly less cache misses
			func(*m_components[i], m_componentInfos[i].isFeedbackComponent());
		}
	}

	/// Iterate all components.
	template<typename TFunct>
	void iterateComponents(TFunct func)
	{
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			// Use the m_componentInfos to check if it's feedback component for possibly less cache misses
			func(*m_components[i], m_componentInfos[i].isFeedbackComponent());
		}
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func) const
	{
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			if(m_componentInfos[i].getComponentClassId() == TComponent::getStaticClassId())
			{
				func(static_cast<const TComponent&>(*m_components[i]));
			}
		}
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func)
	{
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			if(m_componentInfos[i].getComponentClassId() == TComponent::getStaticClassId())
			{
				func(static_cast<TComponent&>(*m_components[i]));
			}
		}
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent* tryGetFirstComponentOfType() const
	{
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			if(m_componentInfos[i].getComponentClassId() == TComponent::getStaticClassId())
			{
				return static_cast<const TComponent*>(m_components[i]);
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
		for(U32 i = 0; i < m_componentInfos.getSize(); ++i)
		{
			if(m_componentInfos[i].getComponentClassId() == TComponent::getStaticClassId() && inth-- == 0)
			{
				return static_cast<const TComponent*>(m_components[i]);
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
		ANKI_ASSERT(m_componentInfos[idx].getComponentClassId() == TComponent::getStaticClassId());
		SceneComponent* c = m_components[idx];
		return *static_cast<TComponent*>(c);
	}

	/// Get the nth component.
	template<typename TComponent>
	const TComponent& getComponentAt(U32 idx) const
	{
		ANKI_ASSERT(m_componentInfos[idx].getComponentClassId() == TComponent::getStaticClassId());
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
		iterateComponentsOfType<TComponent>([&]([[maybe_unused]] const TComponent& c) {
			++count;
		});
		return count;
	}

protected:
	/// Create and append a component to the components container. The SceneNode has the ownership.
	template<typename TComponent>
	TComponent* newComponent()
	{
		TComponent* comp = newInstance<TComponent>(getMemoryPool(), this);
		m_components.emplaceBack(getMemoryPool(), comp);
		m_componentInfos.emplaceBack(getMemoryPool(), *comp);
		return comp;
	}

	ResourceManager& getResourceManager();

private:
	/// This class packs a few info used by components.
	class ComponentsArrayElement
	{
	public:
		ComponentsArrayElement(const SceneComponent& comp)
		{
			m_classId = comp.getClassId() & 0x7F;
			ANKI_ASSERT(m_classId == comp.getClassId());
			m_feedbackComponent = comp.isFeedbackComponent();
		}

		ComponentsArrayElement(const ComponentsArrayElement& b)
			: m_feedbackComponent(b.m_feedbackComponent)
			, m_classId(b.m_classId)
		{
		}

		ComponentsArrayElement& operator=(const ComponentsArrayElement& b)
		{
			m_feedbackComponent = b.m_feedbackComponent;
			m_classId = b.m_classId;
			return *this;
		}

		U8 getComponentClassId() const
		{
			return m_classId;
		}

		Bool isFeedbackComponent() const
		{
			return m_feedbackComponent;
		}

	private:
		U8 m_feedbackComponent : 1;
		U8 m_classId : 7;
	};

	static_assert(sizeof(ComponentsArrayElement) == sizeof(U8), "Wrong size");

	SceneGraph* m_scene = nullptr;
	U64 m_uuid;
	String m_name; ///< A unique name.

	DynamicArray<SceneComponent*> m_components;
	DynamicArray<ComponentsArrayElement> m_componentInfos; ///< Same size as m_components. Used to iterate fast.

	Timestamp m_maxComponentTimestamp = 0;

	Bool m_markedForDeletion = false;
};
/// @}

} // end namespace anki
