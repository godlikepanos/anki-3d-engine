// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/Common.h"
#include "anki/util/Hierarchy.h"
#include "anki/util/Rtti.h"
#include "anki/util/Bitset.h"
#include "anki/util/Enum.h"
#include "anki/scene/SceneComponent.h"

namespace anki {

class SceneGraph; // Don't include
class ResourceManager;

/// @addtogroup scene
/// @{

/// Interface class backbone of scene
class SceneNode: public Hierarchy<SceneNode>
{
public:
	using Base = Hierarchy<SceneNode>;

	/// The one and only constructor
	SceneNode(SceneGraph* scene);

	/// Unregister node
	virtual ~SceneNode();

	/// @param name The unique name of the node. If it's nullptr the the node
	///             is not searchable.
	ANKI_USE_RESULT Error create(const CString& name);

	SceneGraph& getSceneGraph()
	{
		return *m_scene;
	}

	/// Return the name. It may be empty for nodes that we don't want to track
	CString getName() const
	{
		return (!m_name.isEmpty()) ? m_name.toCString() : CString();
	}

	U32 getComponentsCount() const
	{
		return m_componentsCount;
	}

	Bool getMarkedForDeletion() const
	{
		return m_flags.bitsEnabled(Flag::MARKED_FOR_DELETION);
	}

	void setMarkedForDeletion();

	Timestamp getGlobalTimestamp() const;

	SceneAllocator<U8> getSceneAllocator() const;

	SceneFrameAllocator<U8> getFrameAllocator() const;

	void addChild(SceneNode* obj)
	{
		Base::addChild(getSceneAllocator(), obj);
	}

	/// This is called by the scene every frame after logic and before
	/// rendering. By default it does nothing
	/// @param prevUpdateTime Timestamp of the previous update
	/// @param crntTime Timestamp of this update
	virtual ANKI_USE_RESULT Error frameUpdate(F32 prevUpdateTime, F32 crntTime)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		return ErrorCode::NONE;
	}

	/// Return the last frame the node was updated. It checks all components
	U32 getLastUpdateFrame() const;

	/// Inform if a sector has visited this node.
	void setSectorVisited(Bool visited)
	{
		m_flags.enableBits(Flag::SECTOR_VISITED, visited);
	}

	/// Check if a sector has visited this node.
	Bool getSectorVisited() const
	{
		return m_flags.bitsEnabled(Flag::SECTOR_VISITED);
	}

	/// Iterate all components
	template<typename Func>
	ANKI_USE_RESULT Error iterateComponents(Func func) const
	{
		Error err = ErrorCode::NONE;
		auto it = m_components.getBegin();
		auto end = it + m_componentsCount;
		for(; !err && it != end; ++it)
		{
			err = func(*(*it));
		}

		return err;
	}

	/// Iterate all components of a specific type
	template<typename Component, typename Func>
	ANKI_USE_RESULT Error iterateComponentsOfType(Func func)
	{
		Error err = ErrorCode::NONE;
		auto it = m_components.getBegin();
		auto end = it + m_componentsCount;
		for(; !err && it != end; ++it)
		{
			SceneComponent* comp = *it;
			if(isa<Component>(comp))
			{
				err = func(*dcast<Component*>(comp));
			}
		}

		return err;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	Component* tryGetComponent()
	{
		U count = m_componentsCount;
		while(count-- != 0)
		{
			if(isa<Component>(m_components[count]))
			{
				return dcast<Component*>(m_components[count]);
			}
		}
		return nullptr;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	const Component* tryGetComponent() const
	{
		U count = m_componentsCount;
		while(count-- != 0)
		{
			if(isa<Component>(m_components[count]))
			{
				return dcast<const Component*>(m_components[count]);
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

protected:
	/// Append a component to the components container. The SceneNode will not
	/// take ownership
	void addComponent(SceneComponent* comp, Bool transferOwnership = false);

	/// Remove a component from the container
	void removeComponent(SceneComponent* comp);

	ResourceManager& getResourceManager();

private:
	enum class Flag
	{
		MARKED_FOR_DELETION = 1 << 0,
		SECTOR_VISITED = 1 << 1
	};
	ANKI_ENUM_ALLOW_NUMERIC_OPERATIONS(Flag, friend)

	SceneGraph* m_scene = nullptr;

	DArray<SceneComponent*> m_components;
	U8 m_componentsCount = 0;

	String m_name; ///< A unique name
	Bitset<Flag> m_flags;

	void cacheImportantComponents();
};
/// @}

} // end namespace anki

