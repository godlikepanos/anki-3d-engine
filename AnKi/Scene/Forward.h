// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

namespace anki {

// Components
class SceneComponent;
#define ANKI_DEFINE_SCENE_COMPONENT(name, updateOrder) class name##Component;
#include <AnKi/Scene/Components/SceneComponentClasses.defs.h>

// Nodes
class SceneNode;
class LightNode;
class PointLightNode;
class SpotLightNode;
class CameraNode;

// Events
class EventManager;
class Event;

// Other
class SceneGraph;

} // end namespace anki
