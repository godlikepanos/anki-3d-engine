// Copyright (C) 2009-2023, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#define groupshared
#define globallycoherent
#define nointerpolation
#define SV_DISPATCHTHREADID // gl_GlobalInvocationID
#define SV_GROUPINDEX // gl_LocalInvocationIndex
#define SV_GROUPID // gl_WorkGroupID
#define SV_GROUPTHREADID // gl_LocalInvocationID
#define SV_CULLPRIMITIVE
#define SV_VERTEXID
#define SV_POSITION
#define SV_INSTANCEID
#define numthreads(x, y, z) [nodiscard]
#define outputtopology(x) [nodiscard]
#define unroll [nodiscard]
#define loop [nodiscard]
#define branch [nodiscard]
#define out
#define in
#define inout
#define discard return

#define ANKI_BEGIN_NAMESPACE
#define ANKI_END_NAMESPACE
#define ANKI_HLSL 1

#define ANKI_TASK_SHADER
#define ANKI_VERTEX_SHADER
#define ANKI_FRAGMENT_SHADER
#define ANKI_MESH_SHADER
#define ANKI_COMPUTE_SHADER
#define ANKI_CLOSEST_HIT_SHADER

using I8 = int;
using I16 = int;
using I32 = int;
using U8 = unsigned int;
using U16 = unsigned int;
using U32 = unsigned int;
using F32 = float;
using Bool = bool;

struct UVec2
{
	U32 x;
	U32 y;
};

struct UVec3
{
	U32 x;
	U32 y;
	U32 z;
};

struct UVec4
{
	U32 x;
	U32 y;
	U32 z;
	U32 w;
};

struct IVec2
{
	I32 x;
	I32 y;
};

struct IVec3
{
	I32 x;
	I32 y;
	I32 z;
};

struct IVec4
{
	I32 x;
	I32 y;
	I32 z;
	I32 w;
};

struct Vec2
{
	F32 x;
	F32 y;
};

struct Vec3
{
	F32 x;
	F32 y;
	F32 z;
};

struct Vec4
{
	F32 x;
	F32 y;
	F32 z;
	F32 w;
};

struct Mat4
{
	F32 arr[16];
};

struct SamplerState
{
};

struct SamplerComparisonState
{
};

template<typename T>
struct Texture2D
{
	void GetDimensions(U32& width, U32& height);

	void GetDimensions(F32& width, F32& height);

	T SampleLevel(SamplerState sampler, Vec2 uvs, F32 lod, IVec2 offset = {});

	F32 GatherRed(SamplerState sampler, Vec2 uv);
	F32 GatherGreen(SamplerState sampler, Vec2 uv);
	F32 GatherBlue(SamplerState sampler, Vec2 uv);
	F32 GatherAlpha(SamplerState sampler, Vec2 uv);

	T& operator[](UVec2 coords);
};

template<typename T>
using RWTexture2D = Texture2D<T>;

template<typename T>
using Texture3D = Texture2D<T>;

template<typename T>
using RWTexture3D = Texture2D<T>;

template<typename T>
struct StructuredBuffer
{
	T& operator[](U32 index);

	void GetDimensions(U32& length, U32& stride);
};

template<typename T>
struct Buffer
{
	T& operator[](U32 index);
};

template<typename T>
using RWStructuredBuffer = StructuredBuffer<T>;

template<typename T>
struct ConstantBuffer : public T
{
};

struct ByteAddressBuffer
{
	template<typename T>
	T& Load(U32 offset);
};

struct RWByteAddressBuffer
{
	template<typename T>
	T& Load(U32 offset);

	template<typename T>
	void Store(U32 offset, T x);
};

struct RaytracingAccelerationStructure
{
};

// Basic functions

template<typename T>
T min(T a, T b);

template<typename T>
T max(T a, T b);

template<typename T>
T saturate(T a);

template<typename T>
float dot(T a, T b);

template<typename T>
T cross(T a, T b);

template<typename T>
T ddx(T a);

template<typename T>
T ddy(T a);

U32 asuint(float f);

F32 asfloat(U32 u);

I32 asint(float u);

U32 NonUniformResourceIndex(U32 x);

template<typename T>
T rsqrt(T x);

template<typename T>
T round(T x);

template<typename T>
T ceil(T x);

// Atomics

template<typename T>
void InterlockedAdd(T dest, T value, T& originalValue);

template<typename T>
void InterlockedAdd(T dest, T value);

template<typename T>
void InterlockedMin(T dest, T value, T& originalValue);

template<typename T>
void InterlockedMin(T dest, T value);

template<typename T>
void InterlockedMax(T dest, T value, T& originalValue);

template<typename T>
void InterlockedMax(T dest, T value);

template<typename T>
void InterlockedExchange(T dest, T value, T& originalValue);

// Wave ops

template<typename T>
T WaveActiveMin(T value);

template<typename T>
T WaveActiveMax(T value);

bool WaveIsFirstLane();

unsigned WaveActiveCountBits(bool bit);

unsigned WaveGetLaneCount();

// Other

void GroupMemoryBarrierWithGroupSync();

template<typename T>
void DispatchMesh(U32 groupSizeX, U32 groupSizeY, U32 groupSizeZ, T payload);

void SetMeshOutputCounts(U32 vertexCount, U32 primitiveCount);
