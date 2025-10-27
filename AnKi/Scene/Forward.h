// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

namespace anki {

// Components
class SceneComponent;
#define ANKI_DEFINE_SCENE_COMPONENT(name, updateOrder, sceneNodeCanHaveMany, icon) class name##Component;
#include <AnKi/Scene/Components/SceneComponentClasses.def.h>

// Nodes
class SceneNode;

// Events
class EventManager;
class Event;

// Other
class SceneGraph;

} // end namespace anki
