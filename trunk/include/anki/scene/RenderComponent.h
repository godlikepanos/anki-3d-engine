// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_SCENE_RENDER_COMPONENT_H
#define ANKI_SCENE_RENDER_COMPONENT_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/scene/SceneComponent.h"
#include "anki/resource/Material.h"
#include "anki/resource/Model.h"

namespace anki {

/// @addtogroup Scene
/// @{

/// The ID of a buildin material variable
enum class BuildinMaterialVariableId: U8
{
	NO_BUILDIN = 0,
	MVP_MATRIX,
	MV_MATRIX,
	VP_MATRIX,
	NORMAL_MATRIX,
	BILLBOARD_MVP_MATRIX,
	MAX_TESS_LEVEL,
	BLURRING,
	MS_DEPTH_MAP,
	COUNT
};

// Forward
class RenderComponentVariable;

template<typename T>
class RenderComponentVariableTemplate;

/// RenderComponent variable base. Its a visitable
typedef VisitableCommonBase<
	RenderComponentVariable, //< The base
	RenderComponentVariableTemplate<F32>,
	RenderComponentVariableTemplate<Vec2>,
	RenderComponentVariableTemplate<Vec3>,
	RenderComponentVariableTemplate<Vec4>,
	RenderComponentVariableTemplate<Mat3>,
	RenderComponentVariableTemplate<Mat4>,
	RenderComponentVariableTemplate<TextureResourcePointer>>
	RenderComponentVariableVisitable;

/// A wrapper on top of MaterialVariable
class RenderComponentVariable: public RenderComponentVariableVisitable
{
public:
	typedef RenderComponentVariableVisitable Base;

	RenderComponentVariable(const MaterialVariable* mvar);
	virtual ~RenderComponentVariable();

	/// @name Accessors
	/// @{
	BuildinMaterialVariableId getBuildinId() const
	{
		return m_buildinId;
	}

	const char* getName() const
	{
		return m_mvar->getName();
	}

	template<typename T>
	const T* begin() const
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived = 
			static_cast<const RenderComponentVariableTemplate<T>*>(this);
		return derived->begin();
	}

	template<typename T>
	const T* end() const
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived = 
			static_cast<const RenderComponentVariableTemplate<T>*>(this);
		return derived->end();
	}

	template<typename T>
	const T& operator[](U idx) const
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived = 
			static_cast<const RenderComponentVariableTemplate<T>*>(this);
		return (*derived)[idx];
	}

	/// This will trigger copy on write
	template<typename T>
	void setValues(const T* values, U32 size, SceneAllocator<U8> alloc)
	{
		ANKI_ASSERT(Base::isTypeOf<RenderComponentVariableTemplate<T>>());
		auto derived = static_cast<RenderComponentVariableTemplate<T>*>(this);
		derived->setValues(values, size, alloc);
	}

	U32 getArraySize() const
	{
		return m_mvar->getArraySize();
	}

	const GlProgramVariable& getGlProgramVariable() const
	{
		return m_mvar->getGlProgramVariable();
	}

	Bool isInstanced() const
	{
		return m_mvar->isInstanced();
	}
	/// @}

	/// A custom cleanup method
	virtual void cleanup(SceneAllocator<U8> alloc) = 0;

protected:
	const MaterialVariable* m_mvar = nullptr;

private:
	BuildinMaterialVariableId m_buildinId;
};

/// RenderComponent variable. This class should not be visible to other 
/// interfaces except render component
template<typename T>
class RenderComponentVariableTemplate: public RenderComponentVariable
{
public:
	typedef T Type;

	RenderComponentVariableTemplate(const MaterialVariable* mvar)
		: RenderComponentVariable(mvar)
	{
		setupVisitable(this);
	}

	~RenderComponentVariableTemplate()
	{
		ANKI_ASSERT(m_copy == nullptr && "Forgot to delete");
	}

	const T* begin() const
	{
		ANKI_ASSERT((m_mvar->hasValues() || m_copy != nullptr)
			&& "Variable does not have any values");
		return (m_copy) ? m_copy : m_mvar->template begin<T>();
	}

	const T* end() const
	{
		ANKI_ASSERT((m_mvar->hasValues() || m_copy != nullptr)
			&& "Variable does not have any values");
		return (m_copy) ? (m_copy + getArraySize()) : m_mvar->template end<T>();
	}

	const T& operator[](U idx) const
	{
		ANKI_ASSERT((m_mvar->hasValues() || m_copy != nullptr)
			&& "Variable does not have any values");
		ANKI_ASSERT(idx < getArraySize());

		// NOTE Working on GCC is wrong
		if(m_copy)
		{
			return m_copy[idx];
		}
		else
		{
			return m_mvar->template operator[]<T>(idx);
		}
	}

	void setValues(const T* values, U32 size, SceneAllocator<U8> alloc)
	{
		ANKI_ASSERT(size <= m_mvar->getArraySize());
		if(m_copy == nullptr)
		{
			m_copy = alloc.newArray<T>(getArraySize());
		}

		for(U i = 0; i < size; i++)
		{
			m_copy[i] = values[i];
		}
	}

	/// Call that manually
	void cleanup(SceneAllocator<U8> alloc)
	{
		if(m_copy)
		{
			alloc.deleteArray(m_copy, getArraySize());
			m_copy = nullptr;
		}
	}

private:
	T* m_copy = nullptr; ///< Copy of the data
};

/// Rendering data input and output. This is a structure because we don't want
/// to change what buildRendering accepts all the time
class RenderingBuildData
{
public:
	RenderingKey m_key;
	const U8* m_subMeshIndicesArray; ///< @note indices != drawing indices
	U32 m_subMeshIndicesCount;
	GlCommandBufferHandle m_jobs; ///< A job chain 
};

/// RenderComponent interface. Implemented by renderable scene nodes
class RenderComponent: public SceneComponent
{
public:
	typedef SceneVector<RenderComponentVariable*> Variables;

	/// @param node Pass node to steal it's allocator
	RenderComponent(SceneNode* node);

	~RenderComponent();

	/// @name Accessors
	/// @{
	Variables::iterator getVariablesBegin()
	{
		return m_vars.begin();
	}
	Variables::iterator getVariablesEnd()
	{
		return m_vars.end();
	}
	/// @}

	/// Build up the rendering.
	/// Given an array of submeshes that are visible append jobs to the GL
	/// job chain
	virtual void buildRendering(RenderingBuildData& data) = 0;

	/// Access the material
	virtual const Material& getMaterial() = 0;

	/// Information for movables. It's actualy an array of transformations.
	/// @param index The index of the transform to get
	/// @param[out] trf The transform to set
	virtual void getRenderWorldTransform(U index, Transform& trf)
	{
		ANKI_ASSERT(getHasWorldTransforms());
		(void)index;
		(void)trf;
	}

	/// Return true if the renderable has world transforms
	virtual Bool getHasWorldTransforms()
	{
		return false;
	}

	Bool getCastsShadow()
	{
		const Material& mtl = getMaterial();
		return mtl.getShadow() && !mtl.isBlendingEnabled();
	}

	/// Iterate variables using a lambda
	template<typename Func>
	void iterateVariables(Func func)
	{
		for(auto var : m_vars)
		{
			func(*var);
		}
	}

	static constexpr Type getClassType()
	{
		return RENDER_COMPONENT;
	}

protected:
	void init();

private:
	Variables m_vars;
};
/// @}

} // end namespace anki

#endif
