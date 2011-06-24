#ifndef EVENT_MAIN_RENDERER_PPS_HDR_H
#define EVENT_MAIN_RENDERER_PPS_HDR_H

#include "Event.h"


namespace Event {


/// Change the HDR properties
class MainRendererPpsHdr: public Event
{
	public:
		/// Constructor
		MainRendererPpsHdr(float startTime, float duration,
		                   float exposure, uint blurringIterationsNum, float blurringDist);

		/// Copy constructor
		MainRendererPpsHdr(const MainRendererPpsHdr& b);

		/// Copy
		MainRendererPpsHdr& operator=(const MainRendererPpsHdr& b);

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


} // end namespace


#endif
