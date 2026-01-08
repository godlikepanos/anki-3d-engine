// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
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
#include <AnKi/Util/DynamicBitSet.h>

namespace anki {

class SceneNodeRegistryRecord : public GlobalRegistryRecord
{
public:
	using GetClassSizeCallback = U32 (*)();
	using ConstructCallback = void (*)(void*, CString);

	GetClassSizeCallback m_getClassSizeCallback = nullptr;
	ConstructCallback m_constructCallback = nullptr;

	SceneNodeRegistryRecord(const Char* name, GetClassSizeCallback getClassSizeCallback, ConstructCallback constructCallback)
		: GlobalRegistryRecord(name)
		, m_getClassSizeCallback(getClassSizeCallback)
		, m_constructCallback(constructCallback)
	{
	}
};

// It's required on all SceneNode derived classes. It registers some global info required by serialization
#define ANKI_REGISTER_SCENE_NODE_CLASS(className) \
	static U32 getClassSizeCallback() \
	{ \
		return U32(sizeof(className)); \
	} \
	static void constructCallback(void* memory, CString sceneNodeName) \
	{ \
		::new(memory) className(sceneNodeName); \
	} \
	inline static SceneNodeRegistryRecord m_registryRecord{ANKI_STRINGIZE(className), getClassSizeCallback, constructCallback}; \
	const SceneNodeRegistryRecord* getSceneNodeRegistryRecord() const override \
	{ \
		static_assert(std::is_final_v<className>, "ANKI_REGISTER_SCENE_NODE_CLASS only in final classes"); \
		return &m_registryRecord; \
	}

// Used in SceneNode::update()
class SceneNodeUpdateInfo
{
public:
	const Second m_previousTime;
	const Second m_currentTime;
	const Second m_dt;
	const Bool m_paused;

	SceneNodeUpdateInfo(Second prevTime, Second crntTime, Bool paused)
		: m_previousTime(prevTime)
		, m_currentTime(crntTime)
		, m_dt(crntTime - prevTime)
		, m_paused(paused)
	{
	}
};

// Base class of the scene
class SceneNode : private IntrusiveHierarchy<SceneNode>, public IntrusiveListEnabled<SceneNode>
{
	friend class SceneComponent;
	friend class SceneGraph;

	template<typename>
	friend class IntrusiveHierarchy;

public:
	using Base = IntrusiveHierarchy<SceneNode>;

	// The one and only constructor.
	// name: The unique name of the node. If it's empty the the node is not searchable.
	SceneNode(CString name);

	virtual ~SceneNode();

	// Return the name. It may be empty for nodes that we don't want to track.
	CString getName() const
	{
		return (!m_name.isEmpty()) ? m_name.toCString() : "Unnamed";
	}

	// Operation is partially deferred. Scene node won't be searchable until the next frame.
	void setName(CString name);

	U32 getUuid() const
	{
		return m_uuid;
	}

	// Hierarchy manipulation //
	// Changes in the hierarchy are deferred and won't be visible until the next frame

	void addChild(SceneNode* obj)
	{
		ANKI_ASSERT(obj);
		obj->setParent(this);
	}

	U32 getChildrenCount() const
	{
		return m_children.getSize();
	}

	WeakArray<SceneNode*> getChildren()
	{
		return WeakArray(m_children);
	}

	ConstWeakArray<SceneNode*> getChildren() const
	{
		return ConstWeakArray(m_children);
	}

	void setParent(SceneNode* obj);

	SceneNode* getParent() const
	{
		return m_parent;
	}

	// End hierarchy manipulation //

	Bool isMarkedForDeletion() const
	{
		return m_markedForDeletion;
	}

	void markForDeletion();

	// Enable serialization for this node, its components and its children
	void setSerialization(Bool enable)
	{
		m_serialize = enable;
	}

	Bool getSerialization() const
	{
		return m_serialize;
	}

	// Some nodes may need to be updated even on pause
	void setUpdateOnPause(Bool update)
	{
		m_updateOnPause = update;
	}

	Bool getUpdateOnPause() const
	{
		return m_updateOnPause;
	}

	Timestamp getComponentMaxTimestamp() const
	{
		return m_maxComponentTimestamp;
	}

	void setComponentMaxTimestamp(Timestamp maxComponentTimestamp)
	{
		ANKI_ASSERT(maxComponentTimestamp > 0);
		m_maxComponentTimestamp = maxComponentTimestamp;
	}

	// This is called by the scenegraph every frame after all component updates. By default it does nothing.
	virtual void update([[maybe_unused]] SceneNodeUpdateInfo& info)
	{
	}

	// Extra serialization for the derived classes
	virtual Error serialize([[maybe_unused]] SceneSerializer& serializer)
	{
		return Error::kNone;
	}

	virtual const SceneNodeRegistryRecord* getSceneNodeRegistryRecord() const
	{
		return nullptr;
	}

	// Iterate all components.
	template<typename TFunct>
	void iterateComponents(TFunct func) const
	{
		for(U32 i = 0; i < m_components.getSize(); ++i)
		{
			if(func(*m_components[i]) == FunctorContinue::kStop)
			{
				break;
			}
		}
	}

	// Iterate all components.
	template<typename TFunct>
	void iterateComponents(TFunct func)
	{
		for(U32 i = 0; i < m_components.getSize(); ++i)
		{
			if(func(*m_components[i]) == FunctorContinue::kStop)
			{
				break;
			}
		}
	}

	// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func) const
	{
		if(hasComponent<TComponent>())
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getType() == TComponent::kClassType)
				{
					if(func(static_cast<const TComponent&>(*m_components[i])) == FunctorContinue::kStop)
					{
						break;
					}
				}
			}
		}
	}

	// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func)
	{
		if(hasComponent<TComponent>())
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getType() == TComponent::kClassType)
				{
					if(func(static_cast<TComponent&>(*m_components[i])) == FunctorContinue::kStop)
					{
						break;
					}
				}
			}
		}
	}

	// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent* tryGetFirstComponentOfType() const
	{
		if(hasComponent<TComponent>())
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getType() == TComponent::kClassType)
				{
					return static_cast<const TComponent*>(m_components[i]);
				}
			}
		}
		return nullptr;
	}

	// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	TComponent* tryGetFirstComponentOfType()
	{
		const TComponent* c = static_cast<const SceneNode*>(this)->tryGetFirstComponentOfType<TComponent>();
		return const_cast<TComponent*>(c);
	}

	// Get a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent& getFirstComponentOfType() const
	{
		const TComponent* out = tryGetFirstComponentOfType<TComponent>();
		ANKI_ASSERT(out != nullptr);
		return *out;
	}

	// Get a pointer to the first component of the requested type
	template<typename TComponent>
	TComponent& getFirstComponentOfType()
	{
		const TComponent& c = static_cast<const SceneNode*>(this)->getFirstComponentOfType<TComponent>();
		return const_cast<TComponent&>(c);
	}

	// Try geting a pointer to the nth component of the requested type.
	template<typename TComponent>
	const TComponent* tryGetNthComponentOfType(U32 nth) const
	{
		if(hasComponent<TComponent>())
		{
			I32 inth = I32(nth);
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getType() == TComponent::kClassType && inth-- == 0)
				{
					return static_cast<const TComponent*>(m_components[i]);
				}
			}
		}
		return nullptr;
	}

	// Try geting a pointer to the nth component of the requested type.
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

	// Get the nth component.
	template<typename TComponent>
	TComponent& getComponentAt(U32 idx)
	{
		ANKI_ASSERT(m_components[idx]->getType() == TComponent::kClassType);
		SceneComponent* c = m_components[idx];
		return *static_cast<TComponent*>(c);
	}

	// Get the nth component.
	template<typename TComponent>
	const TComponent& getComponentAt(U32 idx) const
	{
		ANKI_ASSERT(m_components[idx]->getType() == TComponent::kClassType);
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

	// Create and append a component to the components container. The SceneNode has the ownership.
	template<typename TComponent>
	TComponent* newComponent();

	template<typename TComponent>
	Bool hasComponent() const
	{
		return !!((1u << SceneComponentTypeMask(TComponent::kClassType)) & m_componentTypeMask);
	}

	SceneComponentTypeMask getSceneComponentMask() const
	{
		return m_componentTypeMask;
	}

	// Movement methods //

	// Ignore parent nodes's transform.
	void setIgnoreParentTransform(Bool ignore)
	{
		m_ignoreParentNodeTransform = ignore;
	}

	const Transform& getLocalTransform() const
	{
		return m_ltrf;
	}

	void setLocalTransform(const Transform& x)
	{
		m_ltrf = x;
		m_localTransformDirty = true;
	}

	void setLocalOrigin(const Vec3& x)
	{
		m_ltrf.setOrigin(x);
		m_localTransformDirty = true;
	}

	Vec3 getLocalOrigin() const
	{
		return m_ltrf.getOrigin().xyz;
	}

	void setLocalRotation(const Mat3& x)
	{
		m_ltrf.setRotation(x);
		m_localTransformDirty = true;
	}

	Mat3 getLocalRotation() const
	{
		return m_ltrf.getRotation().getRotationPart();
	}

	void setLocalScale(const Vec3& x)
	{
		m_ltrf.setScale(x);
		m_localTransformDirty = true;
	}

	Vec3 getLocalScale() const
	{
		return m_ltrf.getScale().xyz;
	}

	const Transform& getWorldTransform() const
	{
		return m_wtrf;
	}

	const Transform& getPreviousWorldTransform() const
	{
		return m_prevWTrf;
	}

	void rotateLocalX(F32 angleRad)
	{
		Mat3x4 r = m_ltrf.getRotation();
		r.rotateXAxis(angleRad);
		m_ltrf.setRotation(r);
		m_localTransformDirty = true;
	}

	void rotateLocalY(F32 angleRad)
	{
		Mat3x4 r = m_ltrf.getRotation();
		r.rotateYAxis(angleRad);
		m_ltrf.setRotation(r);
		m_localTransformDirty = true;
	}

	void rotateLocalZ(F32 angleRad)
	{
		Mat3x4 r = m_ltrf.getRotation();
		r.rotateZAxis(angleRad);
		m_ltrf.setRotation(r);
		m_localTransformDirty = true;
	}

	void moveLocalX(F32 distance)
	{
		Vec3 x_axis = m_ltrf.getRotation().getColumn(0);
		m_ltrf.setOrigin(m_ltrf.getOrigin() + Vec4(x_axis, 0.0f) * distance);
		m_localTransformDirty = true;
	}

	void moveLocalY(F32 distance)
	{
		Vec3 y_axis = m_ltrf.getRotation().getColumn(1);
		m_ltrf.setOrigin(m_ltrf.getOrigin() + Vec4(y_axis, 0.0) * distance);
		m_localTransformDirty = true;
	}

	void moveLocalZ(F32 distance)
	{
		Vec3 z_axis = m_ltrf.getRotation().getColumn(2);
		m_ltrf.setOrigin(m_ltrf.getOrigin() + Vec4(z_axis, 0.0) * distance);
		m_localTransformDirty = true;
	}

	void scale(F32 s)
	{
		m_ltrf.setScale(m_ltrf.getScale() * s);
		m_localTransformDirty = true;
	}

	void lookAtPoint(const Vec4& point)
	{
		m_ltrf = m_ltrf.lookAt(point, Vec4::yAxis());
		m_localTransformDirty = true;
	}

	Bool movedThisFrame() const
	{
		return m_transformUpdatedThisFrame;
	}

	// Returns true if the transform got updated
	ANKI_INTERNAL Bool updateTransform();

	ANKI_INTERNAL Bool isLocalTransformDirty() const
	{
		return m_localTransformDirty;
	}

	// End movement methods //

private:
	class SerializeCommonArgs
	{
	public:
		class
		{
		public:
			// Given an index to the component array find out if the component should be serialized
			Array<SceneDynamicBitSet<U32>, U32(SceneComponentType::kCount)> m_serializableComponentMask;

			Array<U32, U32(SceneComponentType::kCount)> m_componentsToBeSerializedCount = {};
		} m_write;

		class
		{
		public:
			SceneHashMap<U32, SceneNode*> m_sceneNodeUuidToNode;
			SceneHashMap<U32, SceneNode*> m_sceneComponentUuidToNode;
		} m_read;
	};

	SceneString m_name; // A unique name.
	U32 m_uuid = 0;

	// Flags
	Bool m_markedForDeletion : 1 = false;
	Bool m_localTransformDirty : 1 = true;
	Bool m_ignoreParentNodeTransform : 1 = false;
	Bool m_transformUpdatedThisFrame : 1 = true;
	Bool m_serialize : 1 = true;
	Bool m_updateOnPause : 1 = false;

	SceneNode* m_parent = nullptr;
	SceneDynamicArray<SceneNode*> m_children;

	SceneComponentTypeMask m_componentTypeMask = SceneComponentTypeMask::kNone;

	GrDynamicArray<SceneComponent*> m_components;

	Timestamp m_maxComponentTimestamp = 0;

	Transform m_ltrf = Transform::getIdentity(); // The transformation in local space
	Transform m_wtrf = Transform::getIdentity(); // The transformation in world space (local combined with parent's transformation)
	Transform m_prevWTrf = Transform::getIdentity(); // Keep the previous transformation for checking if it moved

	void addComponent(SceneComponent* newc);

	Error serializeCommon(SceneSerializer& serializer, SerializeCommonArgs& args);

	// For the IntrusiveHierarchy interface
	SceneDynamicArray<SceneNode*>& getHierarchyChildren()
	{
		return m_children;
	}

	const SceneDynamicArray<SceneNode*>& getHierarchyChildren() const
	{
		return m_children;
	}

	SceneNode* getHierarchyParent() const
	{
		return m_parent;
	}

	SceneNode*& getHierarchyParent()
	{
		return m_parent;
	}
};

} // end namespace anki
