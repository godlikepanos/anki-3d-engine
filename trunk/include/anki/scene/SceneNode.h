#ifndef ANKI_SCENE_SCENE_NODE_H
#define ANKI_SCENE_SCENE_NODE_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/scene/SceneObject.h"
#include "anki/scene/SceneComponent.h"

namespace anki {

class SceneGraph; // Don't include

/// @addtogroup Scene
/// @{

/// Interface class backbone of scene
class SceneNode: public SceneObject
{
public:
	/// Scene node update type
	enum UpdateType
	{
		SYNC_UPDATE,
		ASYNC_UPDATE
	};

	/// @name Constructors/Destructor
	/// @{

	/// The one and only constructor
	/// @param name The unique name of the node. If it's nullptr the the node
	///             is not searchable
	/// @param scene The scene that will register it
	explicit SceneNode(
		const char* name,
		SceneGraph* scene);

	/// Unregister node
	virtual ~SceneNode();
	/// @}

	/// @name Accessors
	/// @{

	/// Return the name. It may be nullptr for nodes that we don't want to 
	/// track
	const char* getName() const
	{
		return name.length() > 0 ? name.c_str() : nullptr;
	}
	/// @}

	/// This is called by the scene every frame after logic and before
	/// rendering. By default it does nothing
	/// @param[in] prevUpdateTime Timestamp of the previous update
	/// @param[in] crntTime Timestamp of this update
	/// @param[in] uptype The type of this update
	virtual void frameUpdate(F32 prevUpdateTime, F32 crntTime, 
		UpdateType uptype)
	{
		(void)prevUpdateTime;
		(void)crntTime;
		(void)uptype;
	}

	/// Called when a component got updated
	virtual void componentUpdated(SceneComponent& component, 
		SceneComponent::UpdateType uptype)
	{
		(void)component;
		(void)uptype;
	}

	/// Return the last frame the node was updated. It checks all components
	U32 getLastUpdateFrame() const;

	/// Iterate all components
	template<typename Func>
	void iterateComponents(Func func)
	{
		for(auto comp : components)
		{
			func(*comp);
		}
	}

	/// Iterate all components. Const version
	template<typename Func>
	void iterateComponents(Func func) const
	{
		for(auto comp : components)
		{
			func(*comp);
		}
	}

	/// Iterate all components of a specific type
	template<typename Component, typename Func>
	void iterateComponentsOfType(Func func)
	{
		SceneComponent::Type type = Component::getClassType();
		for(auto comp : components)
		{
			if(comp->getType() == type)
			{
				func(comp->downCast<Component>());
			}
		}
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	Component* tryGetComponent()
	{
		SceneComponent::Type type = Component::getClassType();
		for(auto comp : components)
		{
			if(comp->getType() == type)
			{
				return &comp->downCast<Component>();
			}
		}
		return nullptr;
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename Component>
	const Component* tryGetComponent() const
	{
		SceneComponent::Type type = Component::getClassType();
		for(auto comp : components)
		{
			if(comp->getType() == type)
			{
				return &comp->downCast<Component>();
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
		Component* out = tryGetComponent<Component>();
		ANKI_ASSERT(out != nullptr);
		return *out;
	}

	static constexpr SceneObject::Type getClassType()
	{
		return SCENE_NODE_TYPE;
	}

protected:
	/// Append a component to the components container
	void addComponent(SceneComponent* comp);

	/// Remove a component from the container
	void removeComponent(SceneComponent* comp);

private:
	SceneString name; ///< A unique name
	SceneVector<SceneComponent*> components;
};
/// @}

} // end namespace anki

#endif
