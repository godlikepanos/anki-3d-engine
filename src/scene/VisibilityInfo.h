#ifndef VISIBILITY_INFO_H
#define VISIBILITY_INFO_H

#include <deque>
#include "util/Vec.h"


class RenderableNode;
class PointLight;
class SpotLight;


/// The class contains scene nodes, categorized by type, that are visible.
/// The nodes are in separate containers for faster shorting
class VisibilityInfo
{
	public:
		typedef std::deque<const RenderableNode*> RContainer;
		typedef Vec<const PointLight*> PLContainer;
		typedef Vec<SpotLight*> SLContainer;

		VisibilityInfo() {}
		~VisibilityInfo();

		/// @name Accessors
		/// @{
		const RContainer& getVisibleMsRenderableNodes() const {return msRNodes;}
		RContainer& getVisibleMsRenderableNodes() {return msRNodes;}

		const RContainer& getVisibleBsRenderableNodes() const {return bsRNodes;}
		RContainer& getVisibleBsRenderableNodes() {return bsRNodes;}

		const PLContainer& getVisiblePointLights() const {return pLights;}
		PLContainer& getVisiblePointLights() {return pLights;}

		const SLContainer& getVisibleSpotLights() const {return sLights;}
		SLContainer& getVisibleSpotLights() {return sLights;}
		/// @}

	private:
		RContainer msRNodes;
		RContainer bsRNodes;
		PLContainer pLights; ///< Used only for non-light cameras
		SLContainer sLights; ///< Used only for non-light cameras
};


#endif


