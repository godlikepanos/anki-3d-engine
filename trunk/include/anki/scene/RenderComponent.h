#ifndef ANKI_SCENE_RENDER_COMPONENT_H
#define ANKI_SCENE_RENDER_COMPONENT_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/gl/BufferObject.h"
#include "anki/resource/Material.h"
#include "anki/resource/Model.h"
#include <mutex>

namespace anki {

/// @addtogroup Scene
/// @{

/// The ID of a buildin material variable
enum BuildinMaterialVariableId
{
	BMV_NO_BUILDIN = 0,
	BMV_MVP_MATRIX,
	BMV_MV_MATRIX,
	BMV_VP_MATRIX,
	BMV_NORMAL_MATRIX,
	BMV_BILLBOARD_MVP_MATRIX,
	BMV_MAX_TESS_LEVEL,
	BMV_BLURRING,
	BMV_MS_DEPTH_MAP,
	BMV_COUNT
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

	RenderComponentVariable(const MaterialVariable* mvar_);
	virtual ~RenderComponentVariable();

	/// @name Accessors
	/// @{
	BuildinMaterialVariableId getBuildinId() const
	{
		return buildinId;
	}

	const std::string& getName() const
	{
		return mvar->getName();
	}

	template<typename T>
	const T* getValues() const
	{
		ANKI_ASSERT(
			Base::getVariadicTypeId<RenderComponentVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		return static_cast<const RenderComponentVariableTemplate<T>*>(
			this)->get();
	}

	/// This will trigger copy on write
	template<typename T>
	void setValues(const T* values, U32 size, SceneAllocator<U8> alloc)
	{
		ANKI_ASSERT(
			Base::getVariadicTypeId<RenderComponentVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		static_cast<RenderComponentVariableTemplate<T>*>(this)->set(
			values, size, alloc);
	}

	U32 getArraySize() const
	{
		return mvar->getArraySize();
	}
	/// @}

	const ShaderProgramUniformVariable* tryFindShaderProgramUniformVariable(
		const PassLodKey key) const
	{
		return mvar->findShaderProgramUniformVariable(key);
	}

	virtual void cleanup(SceneAllocator<U8> alloc) = 0;

protected:
	const MaterialVariable* mvar = nullptr;

private:
	BuildinMaterialVariableId buildinId;
};

/// RenderComponent variable
template<typename T>
class RenderComponentVariableTemplate: public RenderComponentVariable
{
public:
	typedef T Type;

	RenderComponentVariableTemplate(const MaterialVariable* mvar_)
		: RenderComponentVariable(mvar_)
	{
		setupVisitable(this);
	}

	~RenderComponentVariableTemplate()
	{
		ANKI_ASSERT(copy == nullptr && "Forgot to delete");
	}

	const T* get() const
	{
		ANKI_ASSERT((mvar->hasValues() || copy != nullptr)
			&& "Variable does not have any values");
		return (copy) ? copy : mvar->getValues<T>();
	}

	void set(const T* values, U32 size, SceneAllocator<U8> alloc)
	{
		ANKI_ASSERT(size <= mvar->getArraySize());
		if(copy == nullptr)
		{
			copy = alloc.newArray<T>(getArraySize());
		}
		memcpy(copy, values, sizeof(T) * size);
	}

	/// Call that manualy
	void cleanup(SceneAllocator<U8> alloc)
	{
		if(copy)
		{
			alloc.deleteArray(copy, getArraySize());
			copy = nullptr;
		}
	}

private:
	T* copy = nullptr;
};

/// RenderComponent interface. Implemented by renderable scene nodes
class RenderComponent
{
public:
	typedef SceneVector<RenderComponentVariable*> Variables;

	/// @param node Pass node to steal it's allocator
	RenderComponent(SceneNode* node);

	virtual ~RenderComponent();

	/// Get information for rendering.
	/// Given an array of submeshes that are visible return the correct indices
	/// offsets and counts
	virtual void getRenderingData(
		const PassLodKey& key, 
		const Vao*& vao, const ShaderProgram*& prog,
		const U32* subMeshIndicesArray, U subMeshIndicesCount,
		Array<U32, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesCountArray,
		Array<const void*, ANKI_MAX_MULTIDRAW_PRIMITIVES>& indicesOffsetArray, 
		U32& drawcallCount) const = 0;

	/// Access the material
	virtual const Material& getMaterial() = 0;

	Bool castsShadow()
	{
		return getMaterial().getShadow() && !getMaterial().isBlendingEnabled();
	}

	/// Information for movables. It's actualy an array of transformations.
	virtual const Transform* getRenderWorldTransforms()
	{
		return nullptr;
	}

	/// Number of instances. If greater than 1 then it's instanced
	virtual U32 getRenderInstancesCount()
	{
		return 1;
	}

	/// @name Accessors
	/// @{
	Variables::iterator getVariablesBegin()
	{
		return vars.begin();
	}
	Variables::iterator getVariablesEnd()
	{
		return vars.end();
	}
	/// @}

	/// Iterate variables using a lambda
	template<typename Func>
	void iterateVariables(Func func)
	{
		for(auto var : vars)
		{
			func(*var);
		}
	}

	/// Reset on frame start
	void resetFrame()
	{}

protected:
	/// The derived class needs to call that
	void init();

private:
	Variables vars;
};
/// @}

} // end namespace anki

#endif
