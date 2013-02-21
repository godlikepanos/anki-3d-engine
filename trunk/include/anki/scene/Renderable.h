#ifndef ANKI_SCENE_RENDERABLE_H
#define ANKI_SCENE_RENDERABLE_H

#include "anki/scene/Property.h"
#include "anki/scene/Common.h"
#include "anki/gl/Ubo.h"
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
	BMV_COUNT
};

// Forward
class RenderableVariable;

template<typename T>
class RenderableVariableTemplate;

/// Renderable variable base. Its a visitable
typedef VisitableCommonBase<
	RenderableVariable,
	RenderableVariableTemplate<F32>,
	RenderableVariableTemplate<Vec2>,
	RenderableVariableTemplate<Vec3>,
	RenderableVariableTemplate<Vec4>,
	RenderableVariableTemplate<Mat3>,
	RenderableVariableTemplate<Mat4>,
	RenderableVariableTemplate<TextureResourcePointer>>
	RenderableVariableVisitable;

/// A wrapper on top of MaterialVariable
class RenderableVariable: public RenderableVariableVisitable
{
public:
	typedef RenderableVariableVisitable Base;

	RenderableVariable(const MaterialVariable* mvar_);
	virtual ~RenderableVariable();

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
		ANKI_ASSERT(Base::getVariadicTypeId<RenderableVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		return static_cast<const RenderableVariableTemplate<T>*>(this)->get();
	}

	/// This will trigger copy on write
	template<typename T>
	void setValues(const T* values, U32 size)
	{
		ANKI_ASSERT(Base::getVariadicTypeId<RenderableVariableTemplate<T>>()
			== Base::getVisitableTypeId());
		static_cast<RenderableVariableTemplate<T>*>(this)->set(
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

/// Renderable variable
template<typename T>
class RenderableVariableTemplate: public RenderableVariable
{
public:
	typedef T Type;

	RenderableVariableTemplate(const MaterialVariable* mvar_)
		: RenderableVariable(mvar_)
	{
		setupVisitable(this);
	}

	~RenderableVariableTemplate()
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

/// Renderable interface. Implemented by renderable scene nodes
class Renderable
{
public:
	typedef SceneVector<RenderableVariable*> RenderableVariables;

	Renderable(const SceneAllocator<U8>& alloc);

	virtual ~Renderable();

	/// Access to VAOs
	virtual const ModelPatchBase& getRenderableModelPatchBase() const = 0;

	/// Access the material
	virtual const Material& getRenderableMaterial() const = 0;

	/// Information for movables. It's actualy an array of transformations.
	virtual const Transform* getRenderableWorldTransforms() const
	{
		return nullptr;
	}

	/// Number of instances. If greater than 1 then it's instanced
	virtual U32 getRenderableInstancesCount() const
	{
		return 1;
	}

	/// @name Accessors
	/// @{
	RenderableVariables::iterator getVariablesBegin()
	{
		return vars.begin();
	}
	RenderableVariables::iterator getVariablesEnd()
	{
		return vars.end();
	}

	Ubo& getUbo()
	{
		return ubo;
	}
	/// @}

	U32 getSubMeshesCount() const
	{
		return getRenderableModelPatchBase().getSubMeshesCount();
	}

	/// Set the visibility mask of submeshes. This is called on visibility tests
	void setVisibleSubMeshesMask(const SceneNode* frustumable, U64 mask);

	/// Get the mask to render only the needed submeshess. Called on rendering
	U64 getVisibleSubMeshsMask(const SceneNode& frustumable) const;

	/// Reset on frame start
	void resetFrame()
	{
		perframe = nullptr;
	}

protected:
	/// The derived class needs to call that
	void init(PropertyMap& pmap);

private:
	struct FrustumableMaskPair
	{
		const SceneNode* frustumable;
		U64 mask;
	};

	/// Per frame data
	struct PerFrame
	{
		PerFrame(const SceneAllocator<U8>& frameAlloc)
			: pairs(frameAlloc)
		{
			pairs.reserve(3);
		}

		SceneFrameVector<FrustumableMaskPair> pairs;
	};

	RenderableVariables vars;
	Ubo ubo;
	PerFrame* perframe;
	std::mutex mtx;
};
/// @}

} // end namespace anki

#endif
