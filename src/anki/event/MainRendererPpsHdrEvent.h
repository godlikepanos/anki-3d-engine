// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/event/Event.h>
#include <cstdint>

namespace anki
{

#if 0

/// Change the HDR properties
class MainRendererPpsHdrEvent: public Event
{
public:
	/// Constructor
	MainRendererPpsHdrEvent(float startTime, float duration,
		float exposure, uint32_t blurringIterationsNum, float blurringDist);

	/// Copy constructor
	MainRendererPpsHdrEvent(const MainRendererPpsHdrEvent& b);

	/// Copy
	MainRendererPpsHdrEvent& operator=(const MainRendererPpsHdrEvent& b);

private:
	struct Data
	{
		float exposure; ///< @see Hdr::exposure
		uint32_t blurringIterationsNum; ///< @see Hdr::blurringIterationsNum
		float blurringDist; ///< @see Hdr::blurringDist
	};

	Data originalData; ///< From where do we start
	Data finalData; ///< Where do we want to go

	/// Implements Event::updateSp
	void updateSp(float prevUpdateTime, float crntTime);
};

#endif

} // end namespace
