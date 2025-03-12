// Copyright (C) 2009-present, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

// Irradiance dice utilities

#pragma once

#include <AnKi/Shaders/Common.hlsl>

template<typename T>
struct IrradianceDice
{
	vector<T, 3> m_directions[6u]; // +x, -x, +y, -y, +z, -z
};

template<typename T>
IrradianceDice<T> appendIrradianceDice(IrradianceDice<T> input, vector<T, 3> direction, vector<T, 3> radiance, U32 sampleCount)
{
	const T weight = T(kPi) / (sampleCount / 2u);
	const vector<T, 3> wRadiance = radiance * weight;

	input.m_directions[0] += wRadiance * max(T(0), dot(direction, vector<T, 3>(1, 0, 0)));
	input.m_directions[1] += wRadiance * max(T(0), dot(direction, vector<T, 3>(-1, 0, 0)));
	input.m_directions[2] += wRadiance * max(T(0), dot(direction, vector<T, 3>(0, 1, 0)));
	input.m_directions[3] += wRadiance * max(T(0), dot(direction, vector<T, 3>(0, -1, 0)));
	input.m_directions[4] += wRadiance * max(T(0), dot(direction, vector<T, 3>(0, 0, 1)));
	input.m_directions[5] += wRadiance * max(T(0), dot(direction, vector<T, 3>(0, 0, -1)));

	return input;
}

// Given the value of the 6 faces of the dice and a normal, sample the correct weighted value.
// https://www.shadertoy.com/view/XtcBDB
template<typename T>
vector<T, 3> evaluateIrradianceDice(IrradianceDice<T> dice, vector<T, 3> direction)
{
	const vector<T, 3> axisWeights = direction * direction;
	const vector<T, 3> uv = direction * 0.5 + 0.5;

	vector<T, 3> irradiance = lerp(dice.m_directions[1], dice.m_directions[0], uv.x) * axisWeights.x;
	irradiance += lerp(dice.m_directions[3], dice.m_directions[2], uv.y) * axisWeights.y;
	irradiance += lerp(dice.m_directions[5], dice.m_directions[4], uv.z) * axisWeights.z;

	// Divide by weight
	irradiance /= axisWeights.x + axisWeights.y + axisWeights.z + 0.0001;

	return irradiance;
}

template<typename T>
IrradianceDice<T> lerpIrradianceDice(IrradianceDice<T> a, IrradianceDice<T> b, T f)
{
	[unroll] for(U32 i = 0; i < 6u; ++i)
	{
		a.m_directions[i] = lerp(a.m_directions[i], b.m_directions[i], f);
	}

	return a;
}

template<typename T>
IrradianceDice<T> loadIrradianceDice(Texture3D<Vec4> volumes[6u], SamplerState sampler, Vec3 uvw)
{
	IrradianceDice<T> dice;
	[unroll] for(U32 i = 0; i < 6; ++i)
	{
		dice.m_directions[i] = volumes[i].SampleLevel(sampler, uvw, 0.0).xyz;
	}

	return dice;
}

template<typename T>
IrradianceDice<T> loadIrradianceDice(Texture3D<Vec4> volumes[6u], UVec3 uvw)
{
	IrradianceDice<T> dice;
	[unroll] for(U32 i = 0; i < 6; ++i)
	{
		dice.m_directions[i] = volumes[i][uvw].xyz;
	}

	return dice;
}

template<typename T>
IrradianceDice<T> loadIrradianceDice(RWTexture3D<Vec4> volumes[6u], UVec3 uvw)
{
	IrradianceDice<T> dice;
	[unroll] for(U32 i = 0; i < 6; ++i)
	{
		dice.m_directions[i] = volumes[i][uvw].xyz;
	}

	return dice;
}

template<typename T>
void storeIrradianceDice(IrradianceDice<T> dice, RWTexture3D<Vec4> volumes[6u], UVec3 coords)
{
	[unroll] for(U32 i = 0; i < 6; ++i)
	{
		volumes[i][coords] = vector<T, 4>(dice.m_directions[i], T(0));
	}
}
