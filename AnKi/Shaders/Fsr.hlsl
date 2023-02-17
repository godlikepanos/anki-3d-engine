// Copyright (C) 2009-2022, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma anki hlsl
#pragma anki 16bit
#pragma anki mutator SHARPEN 0 1
#pragma anki mutator FSR_QUALITY 0 1

#include <AnKi/Shaders/Functions.hlsl>

[[vk::binding(0)]] SamplerState g_linearAnyClampSampler;
[[vk::binding(1)]] Texture2D<Vec4> g_tex;
#if defined(ANKI_COMPUTE_SHADER)
[[vk::binding(2)]] RWTexture2D<Vec4> g_outUav;
#endif

struct Uniforms
{
	UVec4 m_fsrConsts0;
	UVec4 m_fsrConsts1;
	UVec4 m_fsrConsts2;
	UVec4 m_fsrConsts3;
	UVec2 m_viewportSize;
	UVec2 m_padding;
};

[[vk::push_constant]] ConstantBuffer<Uniforms> g_uniforms;

// FSR begin
#define A_GPU 1
#define A_HLSL 1
#define A_HALF 1
#include <ThirdParty/FidelityFX/ffx_a.h>

#if SHARPEN
#	define FSR_RCAS_H 1

AH4 FsrRcasLoadH(ASW2 p)
{
	return AH4(g_tex.Load(IVec3(p, 0)));
}

void FsrRcasInputH(inout AH1 r, inout AH1 g, inout AH1 b)
{
	ANKI_MAYBE_UNUSED(r);
	ANKI_MAYBE_UNUSED(g);
	ANKI_MAYBE_UNUSED(b);
}

#else // !SHARPEN
#	define FSR_EASU_H 1

AH4 FsrEasuRH(AF2 p)
{
	return AH4(g_tex.GatherRed(g_linearAnyClampSampler, p));
}

AH4 FsrEasuGH(AF2 p)
{
	return AH4(g_tex.GatherGreen(g_linearAnyClampSampler, p));
}

AH4 FsrEasuBH(AF2 p)
{
	return AH4(g_tex.GatherBlue(g_linearAnyClampSampler, p));
}

AH3 FsrEasuSampleH(AF2 p)
{
	return AH3(g_tex.SampleLevel(g_linearAnyClampSampler, p, 0.0).xyz);
}
#endif

#include <ThirdParty/FidelityFX/ffx_fsr1.h>
// FSR end

#if defined(ANKI_COMPUTE_SHADER)
[numthreads(8, 8, 1)] void main(UVec3 svDispatchThreadId : SV_DISPATCHTHREADID)
#else
Vec3 main(Vec4 svPosition : SV_POSITION) : SV_TARGET0
#endif
{
#if defined(ANKI_COMPUTE_SHADER)
	if(any(svDispatchThreadId >= g_uniforms.m_viewportSize))
	{
		return;
	}

	const UVec2 uv = svDispatchThreadId.xy;
#else
	const UVec2 uv = UVec2(svPosition.xy);
#endif

	HVec3 color;
#if SHARPEN
	FsrRcasH(color.r, color.g, color.b, uv, g_uniforms.m_fsrConsts0);
#elif FSR_QUALITY == 0
	FsrEasuL(color, uv, g_uniforms.m_fsrConsts0, g_uniforms.m_fsrConsts1, g_uniforms.m_fsrConsts2,
			 g_uniforms.m_fsrConsts3);
#else
	FsrEasuH(color, uv, g_uniforms.m_fsrConsts0, g_uniforms.m_fsrConsts1, g_uniforms.m_fsrConsts2,
			 g_uniforms.m_fsrConsts3);
#endif

#if defined(ANKI_COMPUTE_SHADER)
	g_outUav[svDispatchThreadId.xy] = Vec4(color, 0.0);
#else
	return HVec3(color);
#endif
}
