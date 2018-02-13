// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/gr/Common.h>

namespace anki
{

inline Bool stencilTestDisabled(StencilOperation stencilFail,
	StencilOperation stencilPassDepthFail,
	StencilOperation stencilPassDepthPass,
	CompareOperation compare)
{
	return stencilFail == StencilOperation::KEEP && stencilPassDepthFail == StencilOperation::KEEP
		   && stencilPassDepthPass == StencilOperation::KEEP && compare == CompareOperation::ALWAYS;
}

inline Bool blendingDisabled(BlendFactor srcFactorRgb,
	BlendFactor dstFactorRgb,
	BlendFactor srcFactorA,
	BlendFactor dstFactorA,
	BlendOperation opRgb,
	BlendOperation opA)
{
	Bool dontWantBlend = srcFactorRgb == BlendFactor::ONE && dstFactorRgb == BlendFactor::ZERO
						 && srcFactorA == BlendFactor::ONE && dstFactorA == BlendFactor::ZERO
						 && (opRgb == BlendOperation::ADD || opRgb == BlendOperation::SUBTRACT)
						 && (opA == BlendOperation::ADD || opA == BlendOperation::SUBTRACT);
	return dontWantBlend;
}

} // end namespace anki
