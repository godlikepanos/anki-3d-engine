#ifndef VISIBILITY_INFO_H
#define VISIBILITY_INFO_H

#include <vector>


class RenderableNode;
class PointLight;
class SpotLight;


/// The class contains scene nodes, categorized by type, that are visible.
/// The nodes are in separate containers for faster shorting
class VisibilityInfo
{
	public:
		template<typename T>
		class Types
		{
			typedef std::vector<T*> Container;
		};

		/// @name Accessors
		/// @{
		const Types<RenderableNode>::Container&
			getVisibleMsRenderableNodes() const
		{
			return msRNodes;
		}
		Types<RenderableNode>::Container& getVisibleMsRenderableNodes()
		{
			return msRNodes;
		}

		const Types<RenderableNode>::Container&
			getVisibleBsRenderableNodes() const
		{
			return bsRNodes;
		}
		Types<RenderableNode>::Container& getVisibleBsRenderableNodes()
		{
			return bsRNodes;
		}

		const Types<PointLight>::Container& getVisiblePointLights() const
		{
			return pLights;
		}
		Types<PointLight>::Container& getVisiblePointLights()
		{
			return pLights;
		}

		const Types<SpotLight>::Container& getVisibleSpotLights() const
		{
			return sLights;
		}
		Types<SpotLight>::Container& getVisibleSpotLights()
		{
			return sLights;
		}
		/// @}

	protected:
		Types<RenderableNode>::Container msRNodes;
		Types<RenderableNode>::Container bsRNodes;
		Types<PointLight>::Container pLights;
		Types<SpotLight>::Container sLights;
};


#endif
