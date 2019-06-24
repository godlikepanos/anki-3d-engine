// Copyright (C) 2009-2019, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneNode.h>
#include <anki/resource/ParticleEmitterResource.h>
#include <anki/renderer/RenderQueue.h>
#include <anki/collision/Obb.h>
#include <anki/physics/Forward.h>

namespace anki
{

/// @addtogroup scene
/// @{

/// The particle emitter scene node. This scene node emitts
class GpuParticleEmitterNode : public SceneNode
{
public:
	GpuParticleEmitterNode(SceneGraph* scene, CString name);

	~GpuParticleEmitterNode();

	ANKI_USE_RESULT Error init(const CString& filename);

	ANKI_USE_RESULT Error frameUpdate(Second prevUpdateTime, Second crntTime) override;
};
/// @}

} // end namespace anki
