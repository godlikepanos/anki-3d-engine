#ifndef _RENDERERINITIALIZER_H_
#define _RENDERERINITIALIZER_H_

#include "Common.h"

/**
 * A struct to initialize the renderer. It contains a few extra params for the MainRenderer
 */
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
		} hdr;

		// Ssao
		struct Ssao
		{
			bool enabled;
			float renderingQuality;
			float bluringQuality;
		} ssao;
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
