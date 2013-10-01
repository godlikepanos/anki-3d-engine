#ifndef ANKI_SCENE_RENDER_COMPONENT_H
#define ANKI_SCENE_RENDER_COMPONENT_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/gl/BufferObject.h"
#include "anki/resource/Material.h"
#include "anki/resource/Model.h"
#include <mutex>

namespace anki {

class SceneNode;

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
	BMV_BLURRING,
	BMV_MS_DEPTH_MAP,
	BMV_COUNT
};

// Forward
class RenderingComponentVariable;

template<typename T>
class RenderingComponentVariableTemplate;

/// RenderingComponent variable base. Its a visitable
typedef VisitableCommonBase<
	RenderingComponentVariable, //< The base
	RenderingComponentVariableTemplate<F32>,
	RenderingComponentVariableTemplate<Vec2>,
	RenderingComponentVariableTemplate<Vec3>,
	RenderingComponentVariableTemplate<Vec4>,
	RenderingComponentVariableTemplate<Mat3>,
	RenderingComponentVariableTemplate<Mat4>,
	RenderingComponentVariableTemplate<TextureResourcePointer>>
	RenderingComponentVariableVisitable;

/// A wrapper on top of MaterialVariable
class RenderingComponentVariable: public RenderingComponentVariableVisitable
{
public:
	typedef RenderingComponentVariableVisitable Base;

	RenderingComponentVariable(const MaterialVariable* mvar_);
	virtual ~RenderingComponentVariable();

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
			Base::getVariadicTypeId<RenderingComponentVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		return static_cast<const RenderingComponentVariableTemplate<T>*>(
			this)->get();
	}

	/// This will trigger copy on write
	template<typename T>
	void setValues(const T* values, U32 size)
	{
		ANKI_ASSERT(
			Base::getVariadicTypeId<RenderingComponentVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		static_cast<RenderingComponentVariableTemplate<T>*>(this)->set(
			values, size);
	}

	U32 getArraySize() const
	{
		return mvar->getArraySize();
	}
	/// @}

	const ShaderProgramUniformVariable* tryFindShaderProgramUniformVariable(
		const PassLevelKey key) const
	{
		return mvar->findShaderProgramUniformVariable(key);
	}

protected:
	const MaterialVariable* mvar = nullptr;

private:
	BuildinMaterialVariableId buildinId;
};

/// RenderingComponent variable
template<typename T>
class RenderingComponentVariableTemplate: public RenderingComponentVariable
{
public:
	typedef T Type;

	RenderingComponentVariableTemplate(const MaterialVariable* mvar_)
		: RenderingComponentVariable(mvar_)
	{
		setupVisitable(this);
	}

	~RenderingComponentVariableTemplate()
	{
		if(copy)
		{
			propperDelete(copy);
		}
	}

	const T* get() const
	{
		ANKI_ASSERT((mvar->hasValues() || copy != nullptr)
			&& "Variable does not have any values");
		return (copy) ? copy : mvar->getValues<T>();
	}

	void set(const T* values, U32 size)
	{
		ANKI_ASSERT(size <= mvar->getArraySize());
		if(copy == nullptr)
		{
			copy = new T[mvar->getArraySize()];
		}
		memcpy(copy, values, sizeof(T) * size);
	}
private:
	T* copy = nullptr;
};

/// RenderingComponent interface. Implemented by renderable scene nodes
class RenderingComponent
{
public:
	typedef SceneVector<RenderingComponentVariable*> Variables;

	/// @param node Pass note to steal it's allocator
	RenderingComponent(SceneNode* node);

	virtual ~RenderingComponent();

	/// Access to VAOs
	virtual const ModelPatchBase& getRenderingComponentModelPatchBase() = 0;

	/// Access the material
	virtual const Material& getRenderingComponentMaterial() = 0;

	/// Information for movables. It's actualy an array of transformations.
	virtual const Transform* getRenderingComponentWorldTransforms()
	{
		return nullptr;
	}

	/// Number of instances. If greater than 1 then it's instanced
	virtual U32 getRenderingComponentInstancesCount()
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
	void iterateRenderingComponentVariables(Func func)
	{
		for(auto var : vars)
		{
			func(*var);
		}
	}

	U32 getSubMeshesCount()
	{
		return getRenderingComponentModelPatchBase().getSubMeshesCount();
	}

	/// Reset on frame start
	void resetFrame()
	{}

protected:
	/// The derived class needs to call that
	void init(PropertyMap& pmap);

private:
	Variables vars;
};
/// @}

} // end namespace anki

#endif
