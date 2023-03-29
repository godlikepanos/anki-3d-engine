// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
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
class SceneGraphExternalSubsystems;

/// @addtogroup scene
/// @{

/// Interface class backbone of scene
class SceneNode : public Hierarchy<SceneNode>, public IntrusiveListEnabled<SceneNode>
{
	friend class SceneComponent;

public:
	using Base = Hierarchy<SceneNode>;

	/// The one and only constructor.
	/// @param scene The owner scene.
	/// @param name The unique name of the node. If it's empty the the node is not searchable.
	SceneNode(CString name);

	/// Unregister node
	virtual ~SceneNode();

	/// A dummy init for those scene nodes that don't need it.
	Error init()
	{
		return Error::kNone;
	}

	/// Return the name. It may be empty for nodes that we don't want to track.
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

	Timestamp getComponentMaxTimestamp() const
	{
		return m_maxComponentTimestamp;
	}

	void setComponentMaxTimestamp(Timestamp maxComponentTimestamp)
	{
		ANKI_ASSERT(maxComponentTimestamp > 0);
		m_maxComponentTimestamp = maxComponentTimestamp;
	}

	void addChild(SceneNode* obj)
	{
		Base::addChild(SceneMemoryPool::getSingleton(), obj);
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
		for(U32 i = 0; i < m_components.getSize(); ++i)
		{
			func(*m_components[i]);
		}
	}

	/// Iterate all components.
	template<typename TFunct>
	void iterateComponents(TFunct func)
	{
		for(U32 i = 0; i < m_components.getSize(); ++i)
		{
			func(*m_components[i]);
		}
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func) const
	{
		if(m_componentTypeMask & (1 << TComponent::getStaticClassId()))
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getClassId() == TComponent::getStaticClassId())
				{
					func(static_cast<const TComponent&>(*m_components[i]));
				}
			}
		}
	}

	/// Iterate all components of a specific type
	template<typename TComponent, typename TFunct>
	void iterateComponentsOfType(TFunct func)
	{
		if(m_componentTypeMask & (1 << TComponent::getStaticClassId()))
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getClassId() == TComponent::getStaticClassId())
				{
					func(static_cast<TComponent&>(*m_components[i]));
				}
			}
		}
	}

	/// Try geting a pointer to the first component of the requested type
	template<typename TComponent>
	const TComponent* tryGetFirstComponentOfType() const
	{
		if(m_componentTypeMask & (1 << TComponent::getStaticClassId()))
		{
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getClassId() == TComponent::getStaticClassId())
				{
					return static_cast<const TComponent*>(m_components[i]);
				}
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
		if(m_componentTypeMask & (1 << TComponent::getStaticClassId()))
		{
			I32 inth = I32(nth);
			for(U32 i = 0; i < m_components.getSize(); ++i)
			{
				if(m_components[i]->getClassId() == TComponent::getStaticClassId() && inth-- == 0)
				{
					return static_cast<const TComponent*>(m_components[i]);
				}
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
		ANKI_ASSERT(m_components[idx]->getClassId() == TComponent::getStaticClassId());
		SceneComponent* c = m_components[idx];
		return *static_cast<TComponent*>(c);
	}

	/// Get the nth component.
	template<typename TComponent>
	const TComponent& getComponentAt(U32 idx) const
	{
		ANKI_ASSERT(m_components[idx]->getClassId() == TComponent::getStaticClassId());
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

	/// Ignore parent nodes's transform.
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

	void setLocalOrigin(const Vec4& x)
	{
		m_ltrf.setOrigin(x);
		m_localTransformDirty = true;
	}

	const Vec4& getLocalOrigin() const
	{
		return m_ltrf.getOrigin();
	}

	void setLocalRotation(const Mat3x4& x)
	{
		m_ltrf.setRotation(x);
		m_localTransformDirty = true;
	}

	const Mat3x4& getLocalRotation() const
	{
		return m_ltrf.getRotation();
	}

	void setLocalScale(F32 x)
	{
		m_ltrf.setScale(x);
		m_localTransformDirty = true;
	}

	F32 getLocalScale() const
	{
		return m_ltrf.getScale();
	}

	const Transform& getWorldTransform() const
	{
		return m_wtrf;
	}

	const Transform& getPreviousWorldTransform() const
	{
		return m_prevWTrf;
	}

	/// @name Mess with the local transform
	/// @{
	void rotateLocalX(F32 angleRad)
	{
		m_ltrf.getRotation().rotateXAxis(angleRad);
		m_localTransformDirty = true;
	}
	void rotateLocalY(F32 angleRad)
	{
		m_ltrf.getRotation().rotateYAxis(angleRad);
		m_localTransformDirty = true;
	}
	void rotateLocalZ(F32 angleRad)
	{
		m_ltrf.getRotation().rotateZAxis(angleRad);
		m_localTransformDirty = true;
	}
	void moveLocalX(F32 distance)
	{
		Vec3 x_axis = m_ltrf.getRotation().getColumn(0);
		m_ltrf.getOrigin() += Vec4(x_axis, 0.0) * distance;
		m_localTransformDirty = true;
	}
	void moveLocalY(F32 distance)
	{
		Vec3 y_axis = m_ltrf.getRotation().getColumn(1);
		m_ltrf.getOrigin() += Vec4(y_axis, 0.0) * distance;
		m_localTransformDirty = true;
	}
	void moveLocalZ(F32 distance)
	{
		Vec3 z_axis = m_ltrf.getRotation().getColumn(2);
		m_ltrf.getOrigin() += Vec4(z_axis, 0.0) * distance;
		m_localTransformDirty = true;
	}
	void scale(F32 s)
	{
		m_ltrf.getScale() *= s;
		m_localTransformDirty = true;
	}

	void lookAtPoint(const Vec4& point)
	{
		m_ltrf.lookAt(point, Vec4(0.0f, 1.0f, 0.0f, 0.0f));
		m_localTransformDirty = true;
	}
	/// @}

	Bool movedThisFrame() const
	{
		return m_transformUpdatedThisFrame;
	}

	ANKI_INTERNAL Bool updateTransform();

	/// Create and append a component to the components container. The SceneNode has the ownership.
	template<typename TComponent>
	TComponent* newComponent()
	{
		TComponent* comp = newInstance<TComponent>(SceneMemoryPool::getSingleton(), this);
		newComponentInternal(comp);
		return comp;
	}

private:
	U64 m_uuid;
	SceneString m_name; ///< A unique name.

	GrDynamicArray<SceneComponent*> m_components;

	U32 m_componentTypeMask = 0;

	Timestamp m_maxComponentTimestamp = 0;

	/// The transformation in local space.
	Transform m_ltrf = Transform::getIdentity();

	/// The transformation in world space (local combined with parent's transformation).
	Transform m_wtrf = Transform::getIdentity();

	/// Keep the previous transformation for checking if it moved.
	Transform m_prevWTrf = Transform::getIdentity();

	// Flags
	Bool m_markedForDeletion : 1 = false;
	Bool m_localTransformDirty : 1 = true;
	Bool m_ignoreParentNodeTransform : 1 = false;
	Bool m_transformUpdatedThisFrame : 1 = true;

	void newComponentInternal(SceneComponent* newc);
};
/// @}

} // end namespace anki
