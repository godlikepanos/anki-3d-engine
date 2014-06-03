#include "anki/collision/Tests.h"
#include "anki/Math.h"
#include <cstring>

namespace anki {
namespace detail {

//==============================================================================
static F32 calcVectorDot3Internal(const F32* a, const F32* b, U stepA, U stepB)
{
	return a[0] * b[0] + a[stepA] * b[stepB] + a[2 * stepA] * b[2 * stepB];
}

static F32 calcVectorDot313(const F32* a, const F32* b)
{ 
	return calcVectorDot3Internal(a, b, 1, 3); 
}
static F32 calcVectorDot331(const F32* a, const F32* b) 
{ 
	return calcVectorDot3Internal(a, b, 3, 1); 
}
static F32 calcVectorDot333(const F32* a, const F32* b) 
{ 
	return calcVectorDot3Internal(a, b, 3, 3); 
}
static F32 calcVectorDot314(const F32* a, const F32* b) 
{ 
	return calcVectorDot3Internal(a, b, 1, 4); 
}
static F32 calcVectorDot341(const F32* a, const F32* b) 
{ 
	return calcVectorDot3Internal(a, b, 4, 1); 
}
static F32 calcVectorDot344(const F32* a, const F32* b) 
{ 
	return calcVectorDot3Internal(a, b, 4, 4); 
}

static void lineClosestApproach(
	const Vec3& pa, const Vec3& ua,
	const Vec3& pb, const Vec3& ub,
	F32* alpha, F32* beta)
{
	Vec3 p;
	p = pb - pa;
	F32 uaub = ua.dot(ub);
	F32 q1 =  ua.dot(p);
	F32 q2 = -ub.dot(p);
	F32 d = 1.0 - uaub * uaub;

	if(d <= 0.0001)
	{
		*alpha = 0.0;
		*beta = 0.0;
	}
	else 
	{
		d = 1.0 / d;
		*alpha = (q1 + uaub * q2) * d;
		*beta = (uaub * q1 + q2) * d;
	}
}

// find all the intersection points between the 2D rectangle with vertices
// at (+/-h[0],+/-h[1]) and the 2D quadrilateral with vertices (p[0],p[1]),
// (p[2],p[3]),(p[4],p[5]),(p[6],p[7]).
//
// the intersection points are returned as x,y pairs in the 'ret' array.
// the number of intersection points is returned by the function (this will
// be in the range 0 to 8).
static U intersectRectQuad(F32 h[2], F32 p[8], F32 ret[16])
{
	// q (and r) contain nq (and nr) coordinate points for the current (and
	// chopped) polygons
	I nq = 4;
	U nr;
	F32 buffer[16];
	F32* q = p;
	F32* r = ret;

	for(I dir = 0; dir <= 1; dir++) 
	{
		// direction notation: xy[0] = x axis, xy[1] = y axis
		for(I sign=-1; sign <= 1; sign += 2) 
		{
			// chop q along the line xy[dir] = sign*h[dir]
			F32* pq = q;
			F32* pr = r;
			nr = 0;

			for(I i = nq; i > 0; i--) 
			{
				// Go through all points in q and all lines between adjacent 
				// points
				if(sign * pq[dir] < h[dir]) 
				{
					// this point is inside the chopping line
					pr[0] = pq[0];
					pr[1] = pq[1];
					pr += 2;
					nr++;

					if(nr & 8) 
					{
						q = r;
						goto done;
					}
				}

				F32* nextq = (i > 1) ? pq + 2 : q;

				if((sign * pq[dir] < h[dir]) ^ (sign * nextq[dir] < h[dir])) 
				{
					// this line crosses the chopping line
					pr[1 - dir] = pq[1 - dir] + (nextq[1 - dir] - pq[1 - dir]) 
						/ (nextq[dir] - pq[dir]) * (sign * h[dir] - pq[dir]);
					pr[dir] = sign * h[dir];
					pr += 2;
					++nr;
					if(nr & 8) 
					{
						q = r;
						goto done;
					}
				}
				pq += 2;
			}

			q = r;
			r = (q == ret) ? buffer : ret;
			nq = nr;
		}
	}
	
done:
	if(q != ret)
	{
		std::memcpy(ret, q, nr * 2 * sizeof(F32));
	}

	return nr;
}

// given n points in the plane (array p, of size 2*n), generate m points that
// best represent the whole set. the definition of 'best' here is not
// predetermined - the idea is to select points that give good box-box
// collision detection behavior. the chosen point indexes are returned in the
// array iret (of size m). 'i0' is always the first entry in the array.
// n must be in the range [1..8]. m must be in the range [1..n]. i0 must be
// in the range [0..n-1].
static void cullPoints(I n, F32 p[], I m, I i0, I iret[])
{
	// compute the centroid of the polygon in cx,cy
	I i, j;
	F32 a, cx, cy, q;

	if(n == 1) 
	{
		cx = p[0];
		cy = p[1];
	}
	else if(n == 2) 
	{
		cx = 0.5 * (p[0] + p[2]);
		cy = 0.5 * (p[1] + p[3]);
	}
	else 
	{
		a = 0.0;
		cx = 0.0;
		cy = 0.0;

		for (i = 0; i < (n - 1); i++) 
		{
			q = p[i * 2] * p[i * 2 + 3] - p[ i * 2 + 2] * p[ i * 2 + 1];
			a += q;
			cx += q * (p[i * 2] + p[i * 2 + 2]);
			cy += q * (p[i * 2 + 1] + p[i * 2 + 3]);
		}

		q = p[n * 2 - 2] * p[1] - p[0] * p[n * 2 - 1];
		a = 1.0 / (3.0 * (a + q));
		cx = a * (cx + q * (p[n * 2 - 2] + p[0]));
		cy = a * (cy + q * (p[n * 2 - 1] + p[1]));
	}

	// compute the angle of each point w.r.t. the centroid
	F32 A[8];
	for(i = 0; i < n; i++) 
	{
		A[i] = atan2(p[i * 2 + 1] - cy, p[i * 2] - cx);
	}

	// search for points that have angles closest to A[i0] + i*(2*pi/m).
	I avail[8];
	for(i = 0; i < n; i++) 
	{
		avail[i] = 1;
	}

	avail[i0] = 0;
	iret[0] = i0;
	iret++;

	for(j = 1; j < m; j++) 
	{
		a = (F32(j) * (2 * getPi<F32>() / m) + A[i0]);

		if(a > getPi<F32>()) 
		{
			a -= 2.0 * getPi<F32>();
		}

		F32 maxdiff = MAX_F32, diff;
		for(i = 0; i < n; i++) 
		{
			if(avail[i]) 
			{
				diff = abs(A[i] - a);

				if(diff > getPi<F32>()) 
				{
					diff = 2.0 * getPi<F32>() - diff;
				}

				if(diff < maxdiff) 
				{
					maxdiff = diff;
					*iret = i;
				}
			}
		}

		ANKI_ASSERT(maxdiff != MAX_F32);
		avail[*iret] = 0;
		iret++;
	}
}

//==============================================================================
U test(const Obb& a, const Obb& b, CollisionTempVector<ContactPoint>& points)
{
}

} // end namespace detail
} // end namespace anki

