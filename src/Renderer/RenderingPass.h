#ifndef RENDERING_PASS_H
#define RENDERING_PASS_H


class Renderer;
class RendererInitializer;


/// Rendering pass
class RenderingPass
{
	public:
		RenderingPass(Renderer& r_): r(r_) {}

		/// All passes should have an init
		virtual void init(const RendererInitializer& initializer) = 0;

	protected:
		Renderer& r; ///< Know your father
};


#endif
