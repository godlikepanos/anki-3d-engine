// Copyright (C) 2009-2016, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/SceneComponent.h>
#include <anki/resource/Material.h>
#include <anki/util/HashMap.h>

namespace anki
{

/// @addtogroup scene
/// @{

// Forward
class RenderComponentVariable;

template<typename T>
class RenderComponentVariableTemplate;

/// RenderComponent variable base. It's a visitable.
using RenderComponentVariableVisitable = VisitableCommonBase<RenderComponentVariable, // The base
	RenderComponentVariableTemplate<F32>,
	RenderComponentVariableTemplate<Vec2>,
	RenderComponentVariableTemplate<Vec3>,
	RenderComponentVariableTemplate<Vec4>,
	RenderComponentVariableTemplate<Mat3>,
	RenderComponentVariableTemplate<Mat4>,
	RenderComponentVariableTemplate<TextureResourcePtr>>;

/// A wrapper on top of MaterialVariable
class RenderComponentVariable : public RenderComponentVariableVisitable
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
		auto derived = static_cast<const RenderComponentVariableTemplate<T>*>(this);
		return derived->getValue();
	}

	const MaterialVariable& getMaterialVariable() const
	{
		return *m_mvar;
	}

protected:
	const MaterialVariable* m_mvar = nullptr;
};

/// RenderComponent variable. This class should not be visible to other interfaces except render component.
template<typename T>
class RenderComponentVariableTemplate : public RenderComponentVariable
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
	{
	}

	void setValue(const T& value)
	{
		ANKI_ASSERT(isTypeOf<RenderComponentVariableTemplate<T>>());
		ANKI_ASSERT(Base::getMaterialVariable().getBuiltin() == BuiltinMaterialVariableId::NONE);
		m_copy = value;
	}

	const T& getValue() const
	{
		return m_copy;
	}

private:
	T m_copy; ///< Copy of the data
};

/// Rendering data input.
class RenderingBuildInfoIn
{
public:
	RenderingKey m_key;
	const U8* m_subMeshIndicesArray; ///< @note indices != drawing indices
	U32 m_subMeshIndicesCount;
};

/// Rendering data output.
class RenderingBuildInfoOut
{
public:
	ResourceGroupPtr m_resourceGroup;
	Mat4 m_transform;
	union A
	{
		DrawArraysIndirectInfo m_arrays;
		DrawElementsIndirectInfo m_elements;

		A()
			: m_elements() // This is safe because of the nature of structs.
		{
		}
	} m_drawcall;
	Bool8 m_drawArrays = false;
	Bool8 m_hasTransform = false;

	PipelineInitInfo* m_state = nullptr;
	PipelineSubStateBit m_stateMask = PipelineSubStateBit::NONE;

	RenderingBuildInfoOut(PipelineInitInfo* state)
		: m_state(state)
	{
		ANKI_ASSERT(state);
	}

	RenderingBuildInfoOut(const RenderingBuildInfoOut& b)
	{
		*this = b;
	}

	RenderingBuildInfoOut& operator=(const RenderingBuildInfoOut& b)
	{
		m_resourceGroup = b.m_resourceGroup;
		m_transform = b.m_transform;
		m_drawcall.m_elements = b.m_drawcall.m_elements;
		m_drawArrays = b.m_drawArrays;
		m_hasTransform = b.m_hasTransform;
		m_state = b.m_state;
		m_stateMask = b.m_stateMask;

		return *this;
	}
};

/// RenderComponent interface. Implemented by renderable scene nodes
class RenderComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::RENDER;

	using Variables = DynamicArray<RenderComponentVariable*>;

	RenderComponent(SceneNode* node, const Material* mtl);

	~RenderComponent();

	ANKI_USE_RESULT Error init();

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
	virtual ANKI_USE_RESULT Error buildRendering(const RenderingBuildInfoIn& in, RenderingBuildInfoOut& out) const = 0;

	/// Access the material
	const Material& getMaterial() const
	{
		ANKI_ASSERT(m_mtl);
		return *m_mtl;
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

	Bool tryGetPipeline(U64 hash, PipelinePtr& ppline)
	{
		auto it = m_localPplineCache.find(hash);
		if(it != m_localPplineCache.getEnd())
		{
			ppline = *it;
			return true;
		}
		else
		{
			return false;
		}
	}

	void storePipeline(U64 hash, PipelinePtr ppline)
	{
		ANKI_ASSERT(m_localPplineCache.find(hash) == m_localPplineCache.getEnd());
		m_localPplineCache.pushBack(getAllocator(), hash, ppline);
	}

private:
	using Key = U64;

	/// Hash the hash.
	class Hasher
	{
	public:
		U64 operator()(const Key& b) const
		{
			return b;
		}
	};

	/// Hash compare.
	class Compare
	{
	public:
		Bool operator()(const Key& a, const Key& b) const
		{
			return a == b;
		}
	};

	Variables m_vars;
	const Material* m_mtl;

	/// This is an optimization, a local hash of pipelines.
	HashMap<U64, PipelinePtr, Hasher, Compare> m_localPplineCache;
};
/// @}

} // end namespace anki
