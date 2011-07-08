#ifndef R_RENDERING_PASS_H
#define R_RENDERING_PASS_H


namespace R {


class Renderer;
struct RendererInitializer;


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


} // end namespace


#endif
