#ifndef VISIBILITY_INFO_H
#define VISIBILITY_INFO_H

#include <deque>
#include "Util/Vec.h"
#include "Util/Accessors.h"


class RenderableNode;
class PointLight;
class SpotLight;


/// The class contains scene nodes, categorized by type, that are visible.
/// The nodes are in separate containers for faster shorting
class VisibilityInfo
{
	public:
		/// @name Accessors
		/// @{
		GETTER_RW(std::deque<const RenderableNode*>, msRenderableNodes, getVisibleMsRenderableNodes)
		GETTER_RW(std::deque<const RenderableNode*>, bsRenderableNodes, getVisibleBsRenderableNodes)
		GETTER_RW(Vec<const PointLight*>, pointLights, getVisiblePointLights)
		GETTER_RW(Vec<SpotLight*>, spotLights, getVisibleSpotLights)
		/// @}

	private:
		std::deque<const RenderableNode*> msRenderableNodes;
		std::deque<const RenderableNode*> bsRenderableNodes;
		Vec<const PointLight*> pointLights; ///< Used only for non-light cameras
		Vec<SpotLight*> spotLights; ///< Used only for non-light cameras
};


#endif


