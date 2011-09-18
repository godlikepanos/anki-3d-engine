#ifndef MS_H
#define MS_H

#include "RenderingPass.h"
#include "rsrc/Texture.h"
#include "gl/Fbo.h"
#include "util/Accessors.h"
#include "Ez.h"


/// Material stage
class Ms: public RenderingPass
{
	public:
		Ms(Renderer& r_);
		~Ms();

		/// @name Accessors
		/// @{
		const Texture& getNormalFai() const
		{
			return normalFai;
		}

		const Texture& getDiffuseFai() const
		{
			return diffuseFai;
		}

		const Texture& getSpecularFai() const
		{
			return specularFai;
		}

		const Texture& getDepthFai() const
		{
			return depthFai;
		}
		/// @}

		void init(const RendererInitializer& initializer);
		void run();

	private:
		Ez ez; /// EarlyZ pass
		Fbo fbo;
		Texture normalFai; ///< The FAI for normals
		Texture diffuseFai; ///< The FAI for diffuse color
		Texture specularFai; ///< The FAI for specular color and shininess
		Texture depthFai; ///< The FAI for depth
};


#endif
