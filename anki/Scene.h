// Copyright (C) 2009-2020, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/scene/SceneGraph.h>
#include <anki/scene/ModelNode.h>
#include <anki/scene/ParticleEmitterNode.h>
#include <anki/scene/GpuParticleEmitterNode.h>
#include <anki/scene/CameraNode.h>
#include <anki/scene/LightNode.h>
#include <anki/scene/StaticCollisionNode.h>
#include <anki/scene/BodyNode.h>
#include <anki/scene/ReflectionProbeNode.h>
#include <anki/scene/PlayerNode.h>
#include <anki/scene/OccluderNode.h>
#include <anki/scene/DecalNode.h>
#include <anki/scene/Octree.h>
#include <anki/scene/PhysicsDebugNode.h>
#include <anki/scene/TriggerNode.h>
#include <anki/scene/FogDensityNode.h>
#include <anki/scene/GlobalIlluminationProbeNode.h>

#include <anki/scene/components/MoveComponent.h>
#include <anki/scene/components/RenderComponent.h>
#include <anki/scene/components/OccluderComponent.h>
#include <anki/scene/components/LensFlareComponent.h>
#include <anki/scene/components/PlayerControllerComponent.h>
#include <anki/scene/components/SkinComponent.h>
#include <anki/scene/components/ReflectionProbeComponent.h>
#include <anki/scene/components/DecalComponent.h>
#include <anki/scene/components/SceneComponent.h>
#include <anki/scene/components/LightComponent.h>
#include <anki/scene/components/BodyComponent.h>
#include <anki/scene/components/SpatialComponent.h>
#include <anki/scene/components/FrustumComponent.h>
#include <anki/scene/components/JointComponent.h>
#include <anki/scene/components/TriggerComponent.h>
#include <anki/scene/components/FogDensityComponent.h>
#include <anki/scene/components/GlobalIlluminationProbeComponent.h>
#include <anki/scene/components/GenericGpuComputeJobComponent.h>

#include <anki/scene/events/EventManager.h>
#include <anki/scene/events/Event.h>
#include <anki/scene/events/AnimationEvent.h>
#include <anki/scene/events/JitterMoveEvent.h>
#include <anki/scene/events/LightEvent.h>
#include <anki/scene/events/ScriptEvent.h>
