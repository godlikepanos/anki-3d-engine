#ifndef RENDERER_INITIALIZER_H
#define RENDERER_INITIALIZER_H

#include <cstring>


/// A struct to initialize the renderer. It contains a few extra params for
/// the MainRenderer
struct RendererInitializer
{
	// Ms
	struct Ms
	{
		struct Ez
		{
			bool enabled;
		} ez;
	} ms;

	// Is
	struct Is
	{
		// Sm
		struct Sm
		{
			bool enabled;
			bool pcfEnabled;
			bool bilinearEnabled;
			int resolution;
			float level0Distance;
		} sm;
	} is;

	// Pps
	struct Pps
	{
		// Hdr
		struct Hdr
		{
			bool enabled;
			float renderingQuality;
			float blurringDist;
			float blurringIterationsNum;
			float exposure;
		} hdr;

		// Ssao
		struct Ssao
		{
			bool enabled;
			float renderingQuality;
			float blurringIterationsNum;
		} ssao;

		// Bl
		struct Bl
		{
			bool enabled;
			uint blurringIterationsNum;
			float sideBlurFactor;
		} bl;
	} pps;

	// Dbg
	struct Dbg
	{
		bool enabled;
	} dbg;

	// the globals
	int width; ///< Ignored by MainRenderer
	int height; ///< Ignored by MainRenderer
	float mainRendererQuality; ///< Only for MainRenderer

	// funcs
	RendererInitializer() {}

	RendererInitializer(const RendererInitializer& initializer)
	{
		memcpy(this, &initializer, sizeof(RendererInitializer));
	}
};


#endif
