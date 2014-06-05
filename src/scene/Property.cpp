// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include "anki/scene/Property.h"

#if 0
namespace anki {

/// To avoid idiotic mistakes
#define ANKI_DEFINE_PROPERTY_TYPE(SubType_) \
	class SubType_; \
	template<> const uint Property<SubType_>::TYPE_ID = __LINE__;

template<> const uint Property<float>::TYPE_ID = __LINE__;
template<> const uint Property<bool>::TYPE_ID = __LINE__;
ANKI_DEFINE_PROPERTY_TYPE(Vec2)
ANKI_DEFINE_PROPERTY_TYPE(Vec3)
ANKI_DEFINE_PROPERTY_TYPE(OrthographicFrustum)
ANKI_DEFINE_PROPERTY_TYPE(PerspectiveFrustum)

}  // namespace anki
#endif
