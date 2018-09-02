// Copyright (C) 2009-2018, Panagiotis Christopoulos Charitos and contributors.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#include <anki/scene/SoftwareRasterizer.h>
#include <anki/collision/Aabb.h>
#include <anki/core/Trace.h>

namespace anki
{

void SoftwareRasterizer::prepare(const Mat4& mv, const Mat4& p, U width, U height)
{
	m_mv = mv;
	m_p = p;
	m_mvp = p * mv;

	Plane::extractClipPlanes(p, m_planesL);
	Plane::extractClipPlanes(m_mvp, m_planesW);

	// Reset z buffer
	ANKI_ASSERT(width > 0 && height > 0);
	m_width = width;
	m_height = height;
	U size = width * height;
	if(m_zbuffer.getSize() < size)
	{
		m_zbuffer.destroy(m_alloc);
		m_zbuffer.create(m_alloc, size);
	}
	memset(&m_zbuffer[0], 0xFF, sizeof(m_zbuffer[0]) * size);
}

void SoftwareRasterizer::clipTriangle(const Vec4* inVerts, Vec4* outVerts, U& outVertCount) const
{
	ANKI_ASSERT(inVerts && outVerts);

	const Plane& plane = m_planesL[FrustumPlaneType::NEAR];
	F32 clipZ = -plane.getOffset() - EPSILON;
	ANKI_ASSERT(clipZ < 0.0);

	Array<Bool, 3> vertInside;
	U vertInsideCount = 0;
	for(U i = 0; i < 3; ++i)
	{
		vertInside[i] = inVerts[i].z() < clipZ;
		vertInsideCount += (vertInside[i]) ? 1 : 0;
	}

	switch(vertInsideCount)
	{
	case 0:
		// All out
		outVertCount = 0;
		break;
	case 3:
		// All in
		outVertCount = 3;
		outVerts[0] = inVerts[0];
		outVerts[1] = inVerts[1];
		outVerts[2] = inVerts[2];
		break;
	case 1:
	{
		U i, next, prev;
		if(vertInside[0])
		{
			i = 0;
			next = 1;
			prev = 2;
		}
		else if(vertInside[1])
		{
			i = 1;
			next = 2;
			prev = 0;
		}
		else
		{
			i = 2;
			next = 0;
			prev = 1;
		}

		// Find first intersection
		Vec4 rayOrigin = inVerts[i].xyz0();
		Vec4 rayDir = (inVerts[next].xyz0() - rayOrigin).getNormalized();

		Vec4 intersection0;
		Bool intersects = plane.intersectRay(rayOrigin, rayDir, intersection0);
		(void)intersects;
		ANKI_ASSERT(intersects);

		// Find second intersection
		rayDir = (inVerts[prev].xyz0() - rayOrigin).getNormalized();

		Vec4 intersection1;
		intersects = plane.intersectRay(rayOrigin, rayDir, intersection1);
		(void)intersects;
		ANKI_ASSERT(intersects);

		// Finalize
		outVerts[0] = inVerts[i];
		outVerts[1] = intersection0.xyz1();
		outVerts[2] = intersection1.xyz1();
		outVertCount = 3;

		break;
	}
	case 2:
	{
		U in0, in1, out;
		if(vertInside[0] && vertInside[1])
		{
			in0 = 0;
			in1 = 1;
			out = 2;
		}
		else if(vertInside[1] && vertInside[2])
		{
			in0 = 1;
			in1 = 2;
			out = 0;
		}
		else
		{
			ANKI_ASSERT(vertInside[2] && vertInside[0]);
			in0 = 2;
			in1 = 0;
			out = 1;
		}

		// Find first intersection
		Vec4 rayOrigin = inVerts[in1].xyz0();
		Vec4 rayDir = (inVerts[out].xyz0() - rayOrigin).getNormalized();

		Vec4 intersection0;
		Bool intersects = plane.intersectRay(rayOrigin, rayDir, intersection0);
		(void)intersects;
		ANKI_ASSERT(intersects);

		// Find second intersection
		rayOrigin = inVerts[in0].xyz0();
		rayDir = (inVerts[out].xyz0() - rayOrigin).getNormalized();

		Vec4 intersection1;
		intersects = plane.intersectRay(rayOrigin, rayDir, intersection1);
		(void)intersects;
		ANKI_ASSERT(intersects);

		// Two triangles
		outVerts[0] = inVerts[in1];
		outVerts[1] = intersection0;
		outVerts[2] = intersection1;
		outVerts[3] = intersection1;
		outVerts[4] = inVerts[in0];
		outVerts[5] = inVerts[in1];
		outVertCount = 6;

		break;
	}
	}
}

void SoftwareRasterizer::draw(const F32* verts, U vertCount, U stride, Bool backfaceCulling)
{
	ANKI_ASSERT(verts && vertCount > 0 && (vertCount % 3) == 0);
	ANKI_ASSERT(stride >= sizeof(F32) * 3 && (stride % sizeof(F32)) == 0);

	U floatStride = stride / sizeof(F32);
	const F32* vertsEnd = verts + vertCount * floatStride;
	while(verts != vertsEnd)
	{
		// Convert triangle to view space
		Array<Vec4, 3> triVspace;
		for(U j = 0; j < 3; ++j)
		{
			triVspace[j] = m_mv * Vec4(verts[0], verts[1], verts[2], 1.0);
			verts += floatStride;
		}

		// Cull if backfacing
		if(backfaceCulling)
		{
			Vec4 norm = (triVspace[1] - triVspace[0]).cross(triVspace[2] - triVspace[1]);
			ANKI_ASSERT(norm.w() == 0.0f);

			Vec4 eye = triVspace[0].xyz0();
			if(norm.dot(eye) >= 0.0f)
			{
				continue;
			}
		}

		// Clip it
		Array<Vec4, 6> clippedTrisVspace;
		U clippedCount = 0;
		clipTriangle(&triVspace[0], &clippedTrisVspace[0], clippedCount);
		if(clippedCount == 0)
		{
			// Outside view
			continue;
		}

		// Rasterize
		Array<Vec4, 3> clip;
		for(U j = 0; j < clippedCount; j += 3)
		{
			for(U k = 0; k < 3; k++)
			{
				clip[k] = m_p * clippedTrisVspace[j + k].xyz1();
				ANKI_ASSERT(clip[k].w() > 0.0f);
			}

			rasterizeTriangle(&clip[0]);
		}
	}
}

Bool SoftwareRasterizer::computeBarycetrinc(const Vec2& a, const Vec2& b, const Vec2& c, const Vec2& p, Vec3& uvw) const
{
	Vec2 dca = c - a;
	Vec2 dba = b - a;
	Vec2 dap = a - p;

	Vec3 n(dca.x(), dba.x(), dap.x());
	Vec3 m(dca.y(), dba.y(), dap.y());

	Vec3 k = n.cross(m);

	Bool skip = false;
	if(!isZero(k.z()))
	{
		uvw = Vec3(1.0 - (k.x() + k.y()) / k.z(), k.y() / k.z(), k.x() / k.z());

		if(uvw.x() < 0.0f || uvw.y() < 0.0f || uvw.z() < 0.0f)
		{
			skip = true;
		}
	}
	else
	{
		skip = true;
	}

	return skip;
}

void SoftwareRasterizer::rasterizeTriangle(const Vec4* tri)
{
	ANKI_ASSERT(tri);

	const Vec2 windowSize(m_width, m_height);
	Array<Vec3, 3> ndc;
	Array<Vec2, 3> window;
	Vec2 bboxMin(MAX_F32), bboxMax(MIN_F32);
	for(U i = 0; i < 3; i++)
	{
		ndc[i] = tri[i].xyz() / tri[i].w();
		window[i] = (ndc[i].xy() / 2.0 + 0.5) * windowSize;

		for(U j = 0; j < 2; j++)
		{
			bboxMin[j] = floor(min(bboxMin[j], window[i][j]));
			bboxMin[j] = clamp(bboxMin[j], 0.0f, windowSize[j]);

			bboxMax[j] = ceil(max(bboxMax[j], window[i][j]));
			bboxMax[j] = clamp(bboxMax[j], 0.0f, windowSize[j]);
		}
	}

	for(F32 y = bboxMin.y() + 0.5; y < bboxMax.y() + 0.5; y += 1.0)
	{
		for(F32 x = bboxMin.x() + 0.5; x < bboxMax.x() + 0.5; x += 1.0)
		{
			Vec2 p(x, y);
			Vec3 bc;
			if(!computeBarycetrinc(window[0], window[1], window[2], p, bc))
			{
				const F32 z0 = ndc[0].z();
				const F32 z1 = ndc[1].z();
				const F32 z2 = ndc[2].z();

				F32 depth = z0 * bc[0] + z1 * bc[1] + z2 * bc[2];
				ANKI_ASSERT(depth >= 0.0 && depth <= 1.0);

				// Clamp it to a bit less that 1.0f because 1.0f will produce a 0 depthi
				depth = min(depth, 1.0f - EPSILON);

				// Store the min of the current value and new one
				const U32 depthi = depth * MAX_U32;
				m_zbuffer[U(y) * m_width + U(x)].min(depthi);
			}
		}
	}
}

Bool SoftwareRasterizer::visibilityTest(const CollisionShape& cs, const Aabb& aabb) const
{
	ANKI_TRACE_SCOPED_EVENT(SCENE_RASTERIZER_TEST);
	Bool inside = visibilityTestInternal(cs, aabb);

	return inside;
}

Bool SoftwareRasterizer::visibilityTestInternal(const CollisionShape& cs, const Aabb& aabb) const
{
	// Set the AABB points
	const Vec4& minv = aabb.getMin();
	const Vec4& maxv = aabb.getMax();
	Array<Vec4, 8> boxPoints;
	boxPoints[0] = minv.xyz1();
	boxPoints[1] = Vec4(minv.x(), maxv.y(), minv.z(), 1.0f);
	boxPoints[2] = Vec4(minv.x(), maxv.y(), maxv.z(), 1.0f);
	boxPoints[3] = Vec4(minv.x(), minv.y(), maxv.z(), 1.0f);
	boxPoints[4] = maxv.xyz1();
	boxPoints[5] = Vec4(maxv.x(), minv.y(), maxv.z(), 1.0f);
	boxPoints[6] = Vec4(maxv.x(), minv.y(), minv.z(), 1.0f);
	boxPoints[7] = Vec4(maxv.x(), maxv.y(), minv.z(), 1.0f);

	// Transform points
	for(Vec4& p : boxPoints)
	{
		p = m_mvp * p;
	}

	// Check of a point touches the near plane
	for(const Vec4& p : boxPoints)
	{
		if(p.w() <= 0.0f)
		{
			// Don't bother clipping. Just mark it as visible.
			return true;
		}
	}

	// Compute the min and max bounds
	Vec4 bboxMin(MAX_F32);
	Vec4 bboxMax(MIN_F32);
	for(Vec4& p : boxPoints)
	{
		// Perspecrive divide
		p /= p.w();

		// To [0, 1]
		p *= Vec4(0.5f, 0.5f, 1.0f, 1.0f);
		p += Vec4(0.5f, 0.5f, 0.0f, 0.0f);

		// To [0, m_width|m_height]
		p *= Vec4(m_width, m_height, 1.0f, 1.0f);

		// Min
		bboxMin = bboxMin.min(p);

		// Max
		bboxMax = bboxMax.max(p);
	}

	// Fix the bounds
	bboxMin.x() = floorf(bboxMin.x());
	bboxMin.x() = clamp(bboxMin.x(), 0.0f, F32(m_width));

	bboxMax.x() = ceilf(bboxMax.x());
	bboxMax.x() = clamp(bboxMax.x(), 0.0f, F32(m_width));

	bboxMin.y() = floorf(bboxMin.y());
	bboxMin.y() = clamp(bboxMin.y(), 0.0f, F32(m_height));

	bboxMax.y() = ceilf(bboxMax.y());
	bboxMax.y() = clamp(bboxMax.y(), 0.0f, F32(m_height));

	// Loop the tiles
	F32 minZ = bboxMin.z();
	for(U y = bboxMin.y(); y < bboxMax.y(); y += 1.0f)
	{
		for(U x = bboxMin.x(); x < bboxMax.x(); x += 1.0f)
		{
			U idx = U(y) * m_width + U(x);
			U32 depthi = m_zbuffer[idx].get();
			F32 depthf = depthi / F32(MAX_U32);
			if(minZ < depthf)
			{
				return true;
			}
		}
	}

	return false;
}

void SoftwareRasterizer::fillDepthBuffer(ConstWeakArray<F32> depthValues)
{
	ANKI_ASSERT(m_zbuffer.getSize() == depthValues.getSize());

	U count = depthValues.getSize();
	while(count--)
	{
		F32 depth = depthValues[count];
		ANKI_ASSERT(depth >= 0.0f && depth <= 1.0f);

		depth = min(depth, 1.0f - EPSILON); // See a few lines above why is that

		const U32 depthi = depth * MAX_U32;
		m_zbuffer[count].set(depthi);
	}
}

} // end namespace anki
