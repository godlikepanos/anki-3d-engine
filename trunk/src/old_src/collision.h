#ifndef _COLLISION_H_
#define _COLLISION_H_

/*
Info on how seperation tests work.
We have: bvolume1.SeperationTest( bvolume2, normal, impact_point, depth );
This expresion returns the normal, impact_point and depth of the collision between bvolume1 and bvolume2.
The normal shows the direction we have to move bvolume2 in order to seperate the 2 volumes. The depth shows the distance we have to
move bvolume2 for the seperation. The impact_point is a point between inside the collision area.
*/

#include <float.h>
#include "common.h"
#include "math.h"

class lineseg_t;
class ray_t;
class plane_t;
class bsphere_t;
class aabb_t;
class obb_t;


/*
=======================================================================================================================================
bvolume_t (A)                                                                                                                         =
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

		bvolume_t( type_e type_ ): type(type_) {}

		virtual void Render() = 0;

		//if the bounding volume intersects with the plane then returns 0, else the func returns the distance. If the distance is <0 then
		//the b.v. lies behind the plane and if >0 then in front of it
		virtual float PlaneTest( const plane_t& plane ) const = 0;

		virtual bool Intersects( const bvolume_t& bv ) const = 0;
		virtual bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const = 0;
};


/*
=======================================================================================================================================
line segment                                                                                                                          =
=======================================================================================================================================
*/
class lineseg_t: public bvolume_t
{
	public:
		// data members
		vec3_t origin; // P0
		vec3_t dir;    // P1 = origin+dir so dir = P1-origin

		// constructors & distructors
		lineseg_t(): bvolume_t(LINE_SEG) {}
		lineseg_t( const lineseg_t& b ): bvolume_t(LINE_SEG) { (*this)=b; }
		lineseg_t( const vec3_t& origin_, const vec3_t& dir_ ): bvolume_t(LINE_SEG) { origin=origin_; dir=dir_; }

		// operators
		lineseg_t& operator =( const lineseg_t& b ) { origin=b.origin; dir=b.dir; return (*this); }

		// std funcs
		lineseg_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void Render();
		float PlaneTest( const plane_t& plane ) const { DEBUG_ERR("N.A."); return 0.0f; };
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const { DEBUG_ERR("N.A."); return false; };
		bool Intersects( const lineseg_t& segment ) const { DEBUG_ERR("N.A.") return false; };
		bool Intersects( const bsphere_t& sphere ) const;
		bool Intersects( const aabb_t& aabb ) const;
		bool Intersects( const obb_t& obb ) const;
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };

		// other funcs
		/**
		1) If t_c<0 then outside the line segment and close to origin. If t_c>1 again outside the line segment and closer to dir.
		If >0 or <1 then inside the segment
		2) When we talk about distances we calc the distance between the point|line|ray etc and the P0 OR P1. For example the dist
		between a point and P0 or P1 depending of t_c
		*/
		float  LengthSquared() const { return dir.LengthSquared(); }
		float  Length() const { return dir.Length(); }
		float  DistanceSquared( const lineseg_t& seg, float& s_c, float& t_c ) const;   // dist with another segment
		float  DistanceSquared( const ray_t& ray, float& s_c, float& t_c ) const;       // with a ray
		float  DistanceSquared( const vec3_t& point, float& t_c ) const;                // with a point.
		void   ClosestPoints( const lineseg_t& seg, vec3_t& point0, vec3_t& point1 ) const; // closest points between this and another seg
		void   ClosestPoints( const ray_t& ray, vec3_t& point0, vec3_t& point1 ) const;     // with a ray
		vec3_t ClosestPoints( const vec3_t& point ) const;                                  // with a poin
};


/*
=======================================================================================================================================
ray                                                                                                                                   =
=======================================================================================================================================
*/
class ray_t: bvolume_t
{
	public:
		// data members
		vec3_t origin;
		vec3_t dir;

		// constructors & distructors
		ray_t(): bvolume_t(RAY) {}
		ray_t( const ray_t& b ): bvolume_t(RAY) { (*this)=b; }
		ray_t( const vec3_t& origin_, const vec3_t& dir_ ): bvolume_t(RAY), origin(origin_), dir(dir_) {}

		// operators
		ray_t& operator =( const ray_t& b ) { origin=b.origin; dir=b.dir; return (*this); }

		// std funcs
		ray_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void Render();
		float PlaneTest( const plane_t& plane ) const { DEBUG_ERR("N.A."); return 0.0f; };
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const { DEBUG_ERR("N.A."); return false; };
		bool Intersects( const lineseg_t& segment ) const { DEBUG_ERR("N.A.") return false; };
		bool Intersects( const bsphere_t& sphere ) const;
		bool Intersects( const aabb_t& aabb ) const;
		bool Intersects( const obb_t& obb ) const;
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };

		// other funcs
		float  DistanceSquared( const ray_t& ray, float& s_c, float& t_c ) const;   // this and another ray
		void   ClosestPoints( const ray_t& ray, vec3_t& point0, vec3_t& point1 ) const;   // this and another ray
		vec3_t ClosestPoint( const vec3_t& point ) const;                                 // this and point
};


/*
=======================================================================================================================================
plane                                                                                                                                 =
=======================================================================================================================================
*/
class plane_t: public bvolume_t
{
	public:
		// data members
		vec3_t normal;
		float  offset;

		// constructors & distructors
		plane_t(): bvolume_t(PLANE) {}
		plane_t( const plane_t& b ): bvolume_t(PLANE) { (*this)=b; }
		plane_t( const vec3_t& normal_, float offset_ ): bvolume_t(PLANE), normal(normal_), offset(offset_) {}
		plane_t( const vec3_t& p0, const vec3_t& p1, const vec3_t& p2 ): bvolume_t(PLANE) { Set(p0,p1,p2); }
		plane_t( float a, float b, float c, float d ): bvolume_t(PLANE) { Set(a,b,c,d); }

		// operators
		plane_t& operator =( const plane_t& b ) { normal=b.normal; offset=b.offset; return (*this); }

		// std funcs
		plane_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void Render();
		float PlaneTest( const plane_t& plane ) const { DEBUG_ERR("N.A."); return 0.0f; };
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const { DEBUG_ERR("N.A."); return false; };
		bool Intersects( const lineseg_t& segment ) const { DEBUG_ERR("N.A.") return false; };
		bool Intersects( const bsphere_t& sphere ) const{ DEBUG_ERR("N.A.") return false; };
		bool Intersects( const aabb_t& aabb ) const { DEBUG_ERR("N.A.") return false; };
		bool Intersects( const obb_t& obb ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("N.A.") return false; };

		// other funcs
		void Set( const vec3_t& p0, const vec3_t& p1, const vec3_t& p2 ); // set the plane from 3 vectors
		void Set( float a, float b, float c, float d ); // set from plane where plane equation is ax+by+cz+d

		float Test( const vec3_t& point ) const { return normal.Dot(point) - offset; } // it gives the distance between the point and plane.
		                                                                               // if >0 then the point lies in front of the plane,
		                                                                               // if <0 then it is behind and if =0 then it is co-planar
		float Distance( const vec3_t& point ) const { return fabs( Test(point) ); }
		vec3_t ClosestPoint( const vec3_t& point ) const { return point - normal*Test(point); }; // returns the perpedicular of the point in
		                                                                                         // this plane. Plane's normal and returned-point
		                                                                                         // are perpedicular
};


/*
=======================================================================================================================================
bounding sphere                                                                                                                       =
=======================================================================================================================================
*/
class bsphere_t: public bvolume_t
{
	public:
		// data members
		vec3_t center;
		float radius;

		// constructors & distractor
		bsphere_t(): bvolume_t(BSPHERE) {}
		bsphere_t( const bsphere_t& other ): bvolume_t(BSPHERE) { (*this) = other; }
		bsphere_t( const vec3_t& center_, float radius_ ): bvolume_t(BSPHERE), center(center_), radius(radius_) {}

		// operators
		bsphere_t& operator =( const bsphere_t& other ) { center=other.center; radius=other.radius; return (*this); }

		// std funcs
		bsphere_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void Render();
		float PlaneTest( const plane_t& plane ) const;
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const;
		bool Intersects( const lineseg_t& segment ) const;
		bool Intersects( const bsphere_t& sphere ) const;
		bool Intersects( const aabb_t& aabb ) const;
		bool Intersects( const obb_t& obb ) const;
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const;

		// other funcs
		void Set( const void* pointer, uint stride, int count );
};


/*
=======================================================================================================================================
axis aligned bounding box                                                                                                             =
=======================================================================================================================================
*/
class aabb_t: public bvolume_t
{
	public:
		// data members
		vec3_t min;
		vec3_t max;

		// constructors & destractor
		aabb_t(): bvolume_t(AABB) {}
		aabb_t( const aabb_t& other ): bvolume_t(AABB) { (*this) = other; }
		aabb_t( const vec3_t& min_, const vec3_t& max_ ): bvolume_t(AABB), min(min_), max(max_) { DEBUG_ERR( max.x<min.x || max.y<min.y || max.z<min.z ) }

		// operators
		aabb_t& operator =( const aabb_t& other ) { min=other.min; max=other.max; return (*this); }

		// std funcs
		aabb_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void Render();
		float PlaneTest( const plane_t& plane ) const;
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const;
		bool Intersects( const lineseg_t& segment ) const;
		bool Intersects( const bsphere_t& sphere ) const;
		bool Intersects( const aabb_t& aabb ) const;
		bool Intersects( const obb_t& obb ) const;
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("ToDo") return false; };

		// other funcs
		void Set( const void* pointer, uint stride, int count ); // set from vec3 array
};


/*
=======================================================================================================================================
object oriented bounding box                                                                                                          =
=======================================================================================================================================
*/
class obb_t: public bvolume_t
{
	public:
		// data members
		vec3_t center;
		mat3_t rotation;
		vec3_t extends;

		// constructors & destractor
		obb_t(): bvolume_t(OBB) {}
		obb_t( const obb_t& other ): bvolume_t(OBB) { (*this)=other; }
		obb_t( const vec3_t& c_, const mat3_t& r_, const vec3_t& e_ ): bvolume_t(OBB) { center=c_; rotation=r_; extends=e_; }

		// operators
		obb_t& operator =( const obb_t& other ) { center=other.center; rotation=other.rotation; extends=other.extends; return (*this); }

		// std funcs
		obb_t Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const;
		void  Render();
		float PlaneTest( const plane_t& plane ) const;
		bool Intersects( const bvolume_t& bv ) const;
		bool Intersects( const ray_t& ray ) const;
		bool Intersects( const lineseg_t& segment ) const;
		bool Intersects( const bsphere_t& sphere ) const;
		bool Intersects( const aabb_t& aabb ) const;
		bool Intersects( const obb_t& obb ) const;
		bool SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const;
		bool SeperationTest( const aabb_t& aabb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("ToDo") return false; };
		bool SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const { DEBUG_ERR("ToDo") return false; };

		// other funcs
		void Set( const void* pointer, uint stride, int count ); // set from vec3 array
};


#endif
