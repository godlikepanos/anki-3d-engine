// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/resource/Material.h"
#include "anki/resource/Model.h"

namespace anki {

/// @addtogroup scene
/// @{

// Forward
class RenderComponentVariable;

template<typename T>
class RenderComponentVariableTemplate;

/// RenderComponent variable base. It's a visitable.
using RenderComponentVariableVisitable = VisitableCommonBase<
	RenderComponentVariable, // The base
	RenderComponentVariableTemplate<F32>,
	RenderComponentVariableTemplate<Vec2>,
	RenderComponentVariableTemplate<Vec3>,
	RenderComponentVariableTemplate<Vec4>,
	RenderComponentVariableTemplate<Mat3>,
	RenderComponentVariableTemplate<Mat4>,
	RenderComponentVariableTemplate<TextureResourcePtr>>;

/// A wrapper on top of MaterialVariable
class RenderComponentVariable: public RenderComponentVariableVisitable
{
public:
	using Base = RenderComponentVariableVisitable;

	RenderComponentVariable(const MaterialVariable* mvar);
	virtual ~RenderComponentVariable();

	/// This will trigger copy on write
	template<typename T>
	void setValue(const T& value)
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived = static_cast<RenderComponentVariableTemplate<T>*>(this);
		derived->setValue(value);
	}

	template<typename T>
	const T& getValue() const
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived =
			static_cast<const RenderComponentVariableTemplate<T>*>(this);
		return derived->getValue();
	}

	const MaterialVariable& getMaterialVariable() const
	{
		return *m_mvar;
	}

protected:
	const MaterialVariable* m_mvar = nullptr;
};

/// RenderComponent variable. This class should not be visible to other
/// interfaces except render component
template<typename T>
class RenderComponentVariableTemplate: public RenderComponentVariable
{
public:
	using Base = RenderComponentVariable;
	using Type = T;

	RenderComponentVariableTemplate(const MaterialVariable* mvar)
		: RenderComponentVariable(mvar)
	{
		setupVisitable(this);
	}

	~RenderComponentVariableTemplate()
	{}

	void setValue(const T& value)
	{
		ANKI_ASSERT(isTypeOf<RenderComponentVariableTemplate<T>>());
		ANKI_ASSERT(Base::getMaterialVariable().getBuiltin()
			== BuiltinMaterialVariableId::NONE);
		m_copy = value;
	}

	const T& getValue() const
	{
		return m_copy;
	}

private:
	T m_copy; ///< Copy of the data
};

/// Rendering data input and output. This is a structure because we don't want
/// to change what buildRendering accepts all the time
class RenderingBuildInfo
{
public:
	RenderingKey m_key;
	const U8* m_subMeshIndicesArray; ///< @note indices != drawing indices
	U32 m_subMeshIndicesCount;
	CommandBufferPtr m_cmdb; ///< A command buffer to append to.
	DynamicBufferInfo* m_dynamicBufferInfo ANKI_DBG_NULLIFY_PTR;
};

/// RenderComponent interface. Implemented by renderable scene nodes
class RenderComponent: public SceneComponent
{
public:
	using Variables = DArray<RenderComponentVariable*>;

	static Bool classof(const SceneComponent& c)
	{
		return c.getType() == Type::RENDER;
	}

	RenderComponent(SceneNode* node, const Material* mtl, U64 hash = 0);

	~RenderComponent();

	ANKI_USE_RESULT Error create();

	Variables::Iterator getVariablesBegin()
	{
		return m_vars.begin();
	}

	Variables::Iterator getVariablesEnd()
	{
		return m_vars.end();
	}

	Variables::ConstIterator getVariablesBegin() const
	{
		return m_vars.begin();
	}

	Variables::ConstIterator getVariablesEnd() const
	{
		return m_vars.end();
	}

	/// Build up the rendering.
	/// Given an array of submeshes that are visible append jobs to the GL
	/// job chain
	virtual ANKI_USE_RESULT Error buildRendering(
		RenderingBuildInfo& data) const = 0;

	/// Access the material
	const Material& getMaterial() const
	{
		ANKI_ASSERT(m_mtl);
		return *m_mtl;
	}

	/// Information for movables. It's actualy an array of transformations.
	virtual void getRenderWorldTransform(Bool& hasWorldTransforms,
		Transform& trf) const
	{
		hasWorldTransforms = false;
		(void)trf;
	}

	Bool getCastsShadow() const
	{
		const Material& mtl = getMaterial();
		return mtl.getShadowEnabled();
	}

	/// Iterate variables using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateVariables(Func func)
	{
		Error err = ErrorCode::NONE;
		Variables::Iterator it = m_vars.getBegin();
		for(; it != m_vars.getEnd() && !err; it++)
		{
			err = func(*(*it));
		}

		return err;
	}

	Bool canMergeDrawcalls(const RenderComponent& b) const
	{
		return m_mtl->isInstanced() && m_hash != 0 && m_hash == b.m_hash;
	}

private:
	Variables m_vars;
	const Material* m_mtl;

	/// If 2 components have the same hash the renderer may potentially try
	/// to merge them.
	U64 m_hash = 0;
};
/// @}

} // end namespace anki

