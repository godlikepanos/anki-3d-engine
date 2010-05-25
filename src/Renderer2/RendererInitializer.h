#ifndef _RENDERERINITIALIZER_H_
#define _RENDERERINITIALIZER_H_

#include <Common.h>

/**
 * A struct to initialize the renderer. It contains a few extra params for the MainRenderer
 */
struct RendererInitializer
{
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
	int width, height;
};

#endif
