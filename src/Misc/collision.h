#ifndef _COLLISION_H_
#define _COLLISION_H_

/*
Info on how seperation tests work.
We have: bvolume1.SeperationTest(bvolume2, normal, impact_point, depth);
This expresion returns the normal, impact_point and depth of the collision between bvolume1 and bvolume2.
The normal shows the direction we have to move bvolume2 in order to seperate the 2 volumes. The depth shows the distance we have to
move bvolume2 for the seperation. The impact_point is a point between inside the collision area.
*/

#include "Common.h"
#include "Math.h"

class lineseg_t;
class ray_t;
class plane_t;
class bsphere_t;
class aabb_t;
class obb_t;


/*
=======================================================================================================================================
bvolume_t (A)                                                                                                          =
=======================================================================================================================================
*/
class bvolume_t
{
	public:
		enum type_e
		{
			LINE_SEG,
			RAY,
			PLANE,
			BSPHERE,
			AABB,
			OBB,
			BVOLUMES_NUM
		};

		type_e type;

		bvolume_t(type_e type_): type(type_) {}

		virtual void Render() = 0;

		/**
		 * If the bounding volume intersects with the plane then the func returns 0, else it returns the distance. If the distance is <0 then
		 * the b.v. lies behind the plane and if >0 then in front of it
		 */
		virtual float PlaneTest(const plane_t&) const { ERROR("N.A."); return 0.0; }

		// Intersections
		virtual bool Intersects(const lineseg_t&)      const { ERROR("N.A."); return false; }
		virtual bool Intersects(const ray_t&)          const { ERROR("N.A."); return false; }
		virtual bool Intersects(const bsphere_t&)      const { ERROR("N.A."); return false; }
		virtual bool Intersects(const aabb_t&)         const { ERROR("N.A."); return false; }
		virtual bool Intersects(const obb_t&)          const { ERROR("N.A."); return false; }
		        bool Intersects(const bvolume_t& bv)   const;        ///< Abstract intersection test

		// SeperationTests
		virtual bool SeperationTest(const bsphere_t&, Vec3&, Vec3&, float&) const { ERROR("N.A."); return false; }
		virtual bool SeperationTest(const aabb_t&, Vec3&, Vec3&, float&)    const { ERROR("N.A."); return false; }
		virtual bool SeperationTest(const obb_t&, Vec3&, Vec3&, float&)     const { ERROR("N.A."); return false; }
		        bool SeperationTest(const bvolume_t& bv, Vec3& normal, Vec3& impact_point, float& depth) const; ///< Abstract seperation test
};


/*
=======================================================================================================================================
line segment                                                                                                           =
=======================================================================================================================================
*/
class lineseg_t: public bvolume_t
{
	public:
		// data members
		Vec3 origin; // P0
		Vec3 dir;    // P1 = origin+dir so dir = P1-origin

		// constructors & distructors
		lineseg_t(): bvolume_t(LINE_SEG) {}
		lineseg_t(const lineseg_t& b): bvolume_t(LINE_SEG) { (*this)=b; }
		lineseg_t(const Vec3& origin_, const Vec3& dir_): bvolume_t(LINE_SEG) { origin=origin_; dir=dir_; }

		// operators
		lineseg_t& operator =(const lineseg_t& b) { origin=b.origin; dir=b.dir; return (*this); }

		// std funcs
		lineseg_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void Render();
		float PlaneTest(const plane_t&) const;
		bool Intersects(const bsphere_t& sphere) const;
		bool Intersects(const aabb_t& aabb) const;
		bool Intersects(const obb_t& obb) const;

		// other funcs
		/*
		1) If t_c<0 then outside the line segment and close to origin. If t_c>1 again outside the line segment and closer to dir.
		If >0 or <1 then inside the segment
		2) When we talk about distances we calc the distance between the point|line|ray etc and the P0 OR P1. For example the dist
		between a point and P0 or P1 depending of t_c
		*/
		float  LengthSquared() const { return dir.getLengthSquared(); }
		float  Length() const { return dir.getLength(); }
		float  DistanceSquared(const lineseg_t& seg, float& s_c, float& t_c) const;   ///< Dist with another segment
		float  DistanceSquared(const ray_t& ray, float& s_c, float& t_c) const;       ///< Dist with a ray
		float  DistanceSquared(const Vec3& point, float& t_c) const;                ///< Dist with a point.
		void   ClosestPoints(const lineseg_t& seg, Vec3& point0, Vec3& point1) const; ///< Closest points between this and another seg
		void   ClosestPoints(const ray_t& ray, Vec3& point0, Vec3& point1) const;     ///< Closest points between this and a ray
		Vec3 ClosestPoints(const Vec3& point) const;                                  ///< Closest points between this and a poin
};


/*
=======================================================================================================================================
ray                                                                                                                    =
=======================================================================================================================================
*/
class ray_t: bvolume_t
{
	public:
		// data members
		Vec3 origin;
		Vec3 dir; ///< Normalized Vec3

		// constructors & distructors
		ray_t(): bvolume_t(RAY) {}
		ray_t(const ray_t& b): bvolume_t(RAY) { (*this)=b; }
		ray_t(const Vec3& origin_, const Vec3& dir_): bvolume_t(RAY), origin(origin_), dir(dir_) {}

		// operators
		ray_t& operator =(const ray_t& b) { origin=b.origin; dir=b.dir; return (*this); }

		// std funcs
		ray_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void Render();
		float PlaneTest(const plane_t&) const;
		bool Intersects(const bsphere_t& sphere) const;
		bool Intersects(const aabb_t& aabb) const;
		bool Intersects(const obb_t& obb) const;

		// other funcs
		float  DistanceSquared(const ray_t& ray, float& s_c, float& t_c) const;   // this and another ray
		void   ClosestPoints(const ray_t& ray, Vec3& point0, Vec3& point1) const;   // this and another ray
		Vec3 ClosestPoint(const Vec3& point) const;                                 // this and point
};


/*
=======================================================================================================================================
plane                                                                                                                  =
=======================================================================================================================================
*/
class plane_t: public bvolume_t
{
	public:
		// data members
		Vec3 normal;
		float  offset;

		// constructors & distructors
		plane_t(): bvolume_t(PLANE) {}
		plane_t(const plane_t& b): bvolume_t(PLANE) { (*this)=b; }
		plane_t(const Vec3& normal_, float offset_): bvolume_t(PLANE), normal(normal_), offset(offset_) {}
		plane_t(const Vec3& p0, const Vec3& p1, const Vec3& p2): bvolume_t(PLANE) { Set(p0,p1,p2); }
		plane_t(float a, float b, float c, float d): bvolume_t(PLANE) { Set(a,b,c,d); }

		// operators
		plane_t& operator =(const plane_t& b) { normal=b.normal; offset=b.offset; return (*this); }

		// std funcs
		plane_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void Render();

		// other funcs
		void Set(const Vec3& p0, const Vec3& p1, const Vec3& p2); ///< Set the plane from 3 vectors
		void Set(float a, float b, float c, float d); ///< Set from plane where plane equation is ax+by+cz+d

		/**
		 * It gives the distance between a point and a plane. if returns >0 then the point lies in front of the plane, if <0 then it
		 * is behind and if =0 then it is co-planar
		 */
		float Test(const Vec3& point) const { return normal.dot(point) - offset; }

		float Distance(const Vec3& point) const { return fabs(Test(point)); }

		/**
		 * Returns the perpedicular point of a given point in this plane. Plane's normal and returned-point are perpedicular
		 */
		Vec3 ClosestPoint(const Vec3& point) const { return point - normal*Test(point); };
};


/*
=======================================================================================================================================
bounding sphere                                                                                                        =
=======================================================================================================================================
*/
class bsphere_t: public bvolume_t
{
	public:
		// data members
		Vec3 center;
		float radius;

		// constructors & distractor
		bsphere_t(): bvolume_t(BSPHERE) {}
		bsphere_t(const bsphere_t& other): bvolume_t(BSPHERE) { (*this) = other; }
		bsphere_t(const Vec3& center_, float radius_): bvolume_t(BSPHERE), center(center_), radius(radius_) {}

		// operators
		bsphere_t& operator =(const bsphere_t& other) { center=other.center; radius=other.radius; return (*this); }

		// std funcs
		bsphere_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void Render();
		float PlaneTest(const plane_t& plane) const;
		bool Intersects(const ray_t& ray) const;
		bool Intersects(const lineseg_t& segment) const;
		bool Intersects(const bsphere_t& sphere) const;
		bool Intersects(const aabb_t& aabb) const;
		bool Intersects(const obb_t& obb) const;
		bool SeperationTest(const bsphere_t& sphere, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const aabb_t& aabb, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const obb_t& obb, Vec3& normal, Vec3& impact_point, float& depth) const;

		// other funcs
		void Set(const void* pointer, uint stride, int count); ///< Set from vec3 array
};


/*
=======================================================================================================================================
axis aligned bounding box                                                                                              =
=======================================================================================================================================
*/
class aabb_t: public bvolume_t
{
	public:
		// data members
		Vec3 min;
		Vec3 max;

		// constructors & destractor
		aabb_t(): bvolume_t(AABB) {}
		aabb_t(const aabb_t& other): bvolume_t(AABB) { (*this) = other; }
		aabb_t(const Vec3& min_, const Vec3& max_): bvolume_t(AABB), min(min_), max(max_) { DEBUG_ERR(max.x<min.x || max.y<min.y || max.z<min.z); }

		// operators
		aabb_t& operator =(const aabb_t& other) { min=other.min; max=other.max; return (*this); }

		// std funcs
		aabb_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void Render();
		float PlaneTest(const plane_t& plane) const;
		bool Intersects(const ray_t& ray) const;
		bool Intersects(const lineseg_t& segment) const;
		bool Intersects(const bsphere_t& sphere) const;
		bool Intersects(const aabb_t& aabb) const;
		bool Intersects(const obb_t& obb) const;
		bool SeperationTest(const bsphere_t& sphere, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const aabb_t& aabb, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const obb_t& oob, Vec3& normal, Vec3& impact_point, float& depth) const;

		// other funcs
		void Set(const void* pointer, uint stride, int count); ///< Set from vec3 array
};


/*
=======================================================================================================================================
object oriented bounding box                                                                                           =
=======================================================================================================================================
*/
class obb_t: public bvolume_t
{
	public:
		// data members
		Vec3 center;
		Mat3 rotation;
		Vec3 extends;

		// constructors & destractor
		obb_t(): bvolume_t(OBB) {}
		obb_t(const obb_t& other): bvolume_t(OBB) { (*this)=other; }
		obb_t(const Vec3& c_, const Mat3& r_, const Vec3& e_): bvolume_t(OBB) { center=c_; rotation=r_; extends=e_; }

		// operators
		obb_t& operator =(const obb_t& other) { center=other.center; rotation=other.rotation; extends=other.extends; return (*this); }

		// std funcs
		obb_t Transformed(const Vec3& translate, const Mat3& rotate, float scale) const;
		void  Render();
		float PlaneTest(const plane_t& plane) const;
		bool Intersects(const ray_t& ray) const;
		bool Intersects(const lineseg_t& segment) const;
		bool Intersects(const bsphere_t& sphere) const;
		bool Intersects(const aabb_t& aabb) const;
		bool Intersects(const obb_t& obb) const;
		bool SeperationTest(const bvolume_t& bv, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const bsphere_t& sphere, Vec3& normal, Vec3& impact_point, float& depth) const;
		bool SeperationTest(const aabb_t& /*aabb*/, Vec3& /*normal*/, Vec3& /*impact_point*/, float& /*depth*/) const { ERROR("ToDo"); return false; }
		bool SeperationTest(const obb_t& /*obb*/, Vec3& /*normal*/, Vec3& /*impact_point*/, float& /*depth*/) const { ERROR("ToDo"); return false; }

		// other funcs
		void Set(const void* pointer, uint stride, int count); ///< Set from vec3 array
};


/*
=======================================================================================================================================
collision triangle                                                                                                     =
=======================================================================================================================================
*/
/*class cTriangle: public bvolume_t
{
	public:
		// data members
		Vec3 a, b, c;

		// constructors & destractor
		cTriangle() {}
		cTriangle(const Vec3& a_, const Vec3& b_, const Vec3& c_): a(a_), b(b_), c(c_) {}
		cTriangle(const)

		// operators
		cTriangle& operator =(const cTriangle& other) { a=other.a; b=other.b; c=other.c; return (*this); }

		// std funcs
};*/


#endif
