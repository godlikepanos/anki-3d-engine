// Copyright (C) 2009-2015, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_EVENT_LIGHT_EVENT_H
#define ANKI_EVENT_LIGHT_EVENT_H

#include "anki/event/Event.h"
#include "anki/Math.h"

namespace anki {

// Forward
class Light;

/// @addtogroup event
/// @{

/// An event for light animation
class LightEvent: public Event
{
public:
	/// Create
	ANKI_USE_RESULT Error create(
		EventManager* manager, F32 startTime, F32 duration,
		Light* light);

	/// Implements Event::update
	ANKI_USE_RESULT Error update(F32 prevUpdateTime, F32 crntTime);

	void setRadiusMultiplier(F32 v)
	{
		m_radiusMultiplier = v;
	}

	void setIntensityMultiplier(const Vec4& v)
	{
		m_intensityMultiplier = v;
	}

	void setSpecularIntensityMultiplier(const Vec4& v)
	{
		m_specularIntensityMultiplier = v;
	}

private:
	F32 m_radiusMultiplier = 0.0;
	Vec4 m_intensityMultiplier = Vec4(0.0);
	Vec4 m_specularIntensityMultiplier = Vec4(0.0);

	F32 m_originalRadius;
	Vec4 m_originalDiffColor;
	Vec4 m_originalSpecColor;
};
/// @}

} // end namespace anki

#endif
