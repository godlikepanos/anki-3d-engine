// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#if !defined(ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR)
#	define ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
#endif

ANKI_SCENE_COMPONENT_VIRTUAL(constructor, void (*)(SceneComponent& self, SceneNode& owner))
ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
ANKI_SCENE_COMPONENT_VIRTUAL(destructor, void (*)(SceneComponent& self))
ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
ANKI_SCENE_COMPONENT_VIRTUAL(onDestroy, void (*)(SceneComponent& self, SceneNode& owner))
ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
ANKI_SCENE_COMPONENT_VIRTUAL(update, Error (*)(SceneComponent& self, SceneComponentUpdateInfo& info, Bool& updated))
ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
ANKI_SCENE_COMPONENT_VIRTUAL(onOtherComponentRemovedOrAdded,
							 void (*)(SceneComponent& self, SceneComponent* other, Bool added))

#undef ANKI_SCENE_COMPONENT_VIRTUAL
#undef ANKI_SCENE_COMPONENT_VIRTUAL_SEPERATOR
