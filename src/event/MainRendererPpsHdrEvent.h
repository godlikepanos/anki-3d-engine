#ifndef MAIN_RENDERER_PPS_HDR_EVENT_H
#define MAIN_RENDERER_PPS_HDR_EVENT_H

#include "Event.h"


/// Change the HDR properties
class MainRendererPpsHdrEvent: public Event
{
	public:
		/// Constructor
		MainRendererPpsHdrEvent(float startTime, float duration,
			float exposure, uint blurringIterationsNum, float blurringDist);

		/// Copy constructor
		MainRendererPpsHdrEvent(const MainRendererPpsHdrEvent& b);

		/// Copy
		MainRendererPpsHdrEvent& operator=(const MainRendererPpsHdrEvent& b);

	private:
		struct Data
		{
			float exposure; ///< @see Hdr::exposure
			uint blurringIterationsNum; ///< @see Hdr::blurringIterationsNum
			float blurringDist; ///< @see Hdr::blurringDist
		};

		Data originalData; ///< From where do we start
		Data finalData; ///< Where do we want to go

		/// Implements Event::updateSp
		void updateSp(float prevUpdateTime, float crntTime);
};


#endif
