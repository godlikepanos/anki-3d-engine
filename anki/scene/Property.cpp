#include "anki/scene/Property.h"


namespace anki {


/// To avoid idiotic mistakes
#define ANKI_DEFINE_PROPERTY_TYPE(SubType_) \
	class SubType_; \
	template<> const uint Property<SubType_>::TYPE_ID = __LINE__;


template<> const uint Property<float>::TYPE_ID = __LINE__;
ANKI_DEFINE_PROPERTY_TYPE(Vec2)
ANKI_DEFINE_PROPERTY_TYPE(Vec3)
ANKI_DEFINE_PROPERTY_TYPE(OrthographicFrustum)
ANKI_DEFINE_PROPERTY_TYPE(PerspectiveFrustum)


}  // namespace anki
