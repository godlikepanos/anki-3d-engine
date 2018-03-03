// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/Common.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/resource/MaterialResource.h>
#include <anki/core/StagingGpuMemoryManager.h>
#include <anki/renderer/RenderQueue.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// A wrapper on top of MaterialVariable
class RenderComponentVariable
{
	friend class RenderComponent;

public:
	RenderComponentVariable() = default;
	~RenderComponentVariable() = default;

	template<typename T>
	const T& getValue() const
	{
		return m_mvar->getValue<T>();
	}

	const MaterialVariable& getMaterialVariable() const
	{
		return *m_mvar;
	}

private:
	const MaterialVariable* m_mvar = nullptr;
};

/// RenderComponent interface. Implemented by renderable scene nodes
class RenderComponent : public SceneComponent
{
public:
	static const SceneComponentType CLASS_TYPE = SceneComponentType::RENDER;

	using Variables = DynamicArray<RenderComponentVariable>;

	RenderComponent(SceneNode* node, MaterialResourcePtr mtl);

	~RenderComponent();

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

	/// Access the material
	const MaterialResource& getMaterial() const
	{
		ANKI_ASSERT(m_mtl);
		return *m_mtl;
	}

	Bool getCastsShadow() const
	{
		const MaterialResource& mtl = getMaterial();
		return mtl.castsShadow();
	}

	/// Iterate variables using a lambda
	template<typename Func>
	ANKI_USE_RESULT Error iterateVariables(Func func)
	{
		Error err = Error::NONE;
		Variables::Iterator it = m_vars.getBegin();
		for(; it != m_vars.getEnd() && !err; it++)
		{
			err = func(*it);
		}

		return err;
	}

	void allocateAndSetupUniforms(U set,
		const RenderQueueDrawContext& ctx,
		ConstWeakArray<Mat4> transforms,
		StagingGpuMemoryManager& alloc) const;

	virtual void setupRenderableQueueElement(RenderableQueueElement& el) const = 0;

private:
	Variables m_vars;
	MaterialResourcePtr m_mtl;
};
/// @}

} // end namespace anki
