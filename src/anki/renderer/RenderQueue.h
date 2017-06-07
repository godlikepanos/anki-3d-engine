// Copyright (C) 2009-2017, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#pragma once

#include <anki/renderer/Common.h>
#include <anki/core/Timestamp.h>
#include <anki/resource/RenderingKey.h>

namespace anki
{

// Forward
class RenderQueueElement;
class PointLightQueueElement;
class SpotLightQueueElement;
class ReflectionProbeQueueElement;

template<typename T>
class TRenderQueue;

/// @addtogroup renderer
/// @{

using RenderQueue = TRenderQueue<RenderQueueElement>;
using PointLightQueue = TRenderQueue<PointLightQueueElement>;
using SpotLightQueue = TRenderQueue<SpotLightQueueElement>;
using ReflectionProbeQueue = TRenderQueue<ReflectionProbeQueueElement>;

class RenderingMatrices
{
public:
	Mat4 m_cameraTransform;
	Mat4 m_viewMatrix;
	Mat4 m_projectionMatrix;
	Mat4 m_viewProjectionMatrix;
};

class RenderQueueDrawContext final : public RenderingMatrices
{
public:
	RenderingKey m_key;
	CommandBufferPtr m_cmdb;
};

/// Draw callback.
using RenderQueueElementDrawCallback = void (*)(
	const RenderQueueDrawContext& ctx, RenderQueueElement* elements, U elementCount);

class RenderQueueElement final
{
public:
	RenderQueueElementDrawCallback m_callback;
	void* m_userData;
	U64 m_mergeKey;
	F32 m_distanceFromCamera;
};

class PointLightQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	Array<RenderQueue*, 6> m_shadowRenderQueues;
};

class SpotLightQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_distance;
	F32 m_outerAngleCos;
	F32 m_innerAngleCos;
	Vec3 m_diffuseColor;
	Vec3 m_specularColor;
	RenderQueue* m_shadowRenderQueue;
};

class ReflectionProbeQueueElement final
{
public:
	U64 m_uuid;
	Vec3 m_worldPosition;
	F32 m_radius;
	Array<RenderQueue*, 6> m_renderQueues;
};

template<typename T>
class TRenderQueue final : public RenderingMatrices
{
public:
	static constexpr U32 INITIAL_STORAGE_SIZE = 32;
	static constexpr U32 STORAGE_GROW_RATE = 4;

	Timestamp m_lastUpdateTimestamp;

	T* newElement(StackAllocator<T> alloc);

	void mergeBack(StackAllocator<T> alloc, TRenderQueue& b);

private:
	T* m_elements = nullptr;
	U32 m_elementCount = 0;
	U32 m_elementStorage = 0;
};

template<typename T>
inline T* TRenderQueue<T>::newElement(StackAllocator<T> alloc)
{
	if(ANKI_UNLIKELY(m_elementCount + 1 > m_elementStorage))
	{
		m_elementStorage = max(INITIAL_STORAGE_SIZE, m_elementStorage * STORAGE_GROW_RATE);

		const T* oldElements = m_elements;
		m_elements = alloc.allocate(m_elementStorage);

		if(oldElements)
		{
			memcpy(m_elements, oldElements, sizeof(T) * m_elementCount);
		}

		return m_elements[m_elementCount++];
	}

	return m_elements[m_elementCount++];
}

template<typename T>
inline void TRenderQueue<T>::mergeBack(StackAllocator<T> alloc, TRenderQueue& b)
{
	if(b.m_elementCount == 0)
	{
		return;
	}

	if(m_elementCount == 0)
	{
		*this = b;
		b = {};
		return;
	}

	const U32 newElementCount = m_elementCount + b.m_elementCount;

	if(newElementCount > m_elementStorage)
	{
		// Grow storage
		m_elementStorage = newElementCount;

		T* newElements = alloc.allocate(m_elementStorage);

		memcpy(newElements, m_elements, sizeof(T) * m_elementCount);
		m_elements = newElements;
	}

	memcpy(m_elements + m_elementCount, b.m_elements, sizeof(T) * b.m_elementCount);
	m_elementCount = newElementCount;

	b = {};
}

/// The combination of all the results.
class CombinedRenderQueues
{
public:
	RenderQueue m_renderables; ///< Deferred shading or shadow renderables.
	RenderQueue m_forwardShadingRenderables;
	PointLightQueue m_pointLights;
	SpotLightQueue m_spotLights;
	ReflectionProbeQueue m_reflectionProbes;
};
/// @}

} // end namespace anki
