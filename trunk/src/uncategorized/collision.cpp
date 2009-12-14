#include <float.h>
#include "collision.h"
#include "renderer.h"


static int render_seperation_lock = 0;
#define LOCK_RENDER_SEPERATION ++render_seperation_lock;
#define UNLOCK_RENDER_SEPERATION --render_seperation_lock;
#define RENDER_SEPERATION_TEST if(render_seperation_lock==0) RenderSeparationData( normal, impact_point, depth );


/*
=======================================================================================================================================
misc                                                                                                                                  =
=======================================================================================================================================
*/
static void RenderSeparationData( const vec3_t& normal, const vec3_t& impact_point, float depth )
{
	float con = 0.5f;
	const vec3_t& i = impact_point;

	//glLineWidth( 2.0 );
	glBegin( GL_LINES );
		glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
		glVertex3f( i.x-con, i.y, i.z );
		glVertex3f( i.x+con, i.y, i.z );
		glColor3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
		glVertex3f( i.x, i.y-con, i.z );
		glVertex3f( i.x, i.y+con, i.z );
		glColor3fv( &vec3_t( 0.0, 0.0, 1.0 )[0] );
		glVertex3f( i.x, i.y, i.z-con );
		glVertex3f( i.x, i.y, i.z+con );
	glEnd();

	//glLineWidth( 6.0 );
	glBegin( GL_LINES );
		glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
		glVertex3fv( &((vec3_t&)impact_point)[0] );
		glVertex3fv( &(impact_point+ normal*depth )[0] );
	glEnd();
}



/*
=======================================================================================================================================
Intersects                                                                                                                            =
=======================================================================================================================================
*/
bool bvolume_t::Intersects( const bvolume_t& bv ) const
{
	switch( type )
	{
		case LINE_SEG:
			return Intersects( (const lineseg_t&)bv );
		case RAY:
			return Intersects( (const ray_t&)bv );
		case PLANE:
			return PlaneTest( (const plane_t&)bv ) == 0.0;
		case BSPHERE:
			return Intersects( (const bsphere_t&)bv );
		case AABB:
			return Intersects( (const aabb_t&)bv );
		case OBB:
			return Intersects( (const obb_t&)bv );
		default:
			FATAL( "Incorect type" );
	}
	return false;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
=======================================================================================================================================
*/
bool bvolume_t::SeperationTest( const bvolume_t& bv, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	switch( type )
	{
		case BSPHERE:
			return SeperationTest( (const bsphere_t&)bv, normal, impact_point, depth );
		case AABB:
			return SeperationTest( (const aabb_t&)bv, normal, impact_point, depth );
		case OBB:
			return SeperationTest( (const obb_t&)bv, normal, impact_point, depth );
		default:
			FATAL( "Trying to seperate incompatible volumes" );
	}
	return false;
}


/*
=======================================================================================================================================
DistanceSquared                                                                                                                       =
seg - seg                                                                                                                             =
=======================================================================================================================================
*/
float lineseg_t::DistanceSquared( const lineseg_t& ls, float& s_c, float& t_c ) const
{
	// compute intermediate parameters
	vec3_t w0 = origin - ls.origin;
	float a = dir.Dot( dir );
	float b = dir.Dot( ls.dir );
	float c = ls.dir.Dot( ls.dir );
	float d = dir.Dot( w0 );
	float e = ls.dir.Dot( w0 );

	float denom = a*c - b*b;
	// parameters to compute s_c, t_c
	float sn, sd, tn, td;

	// if denom is zero, try finding closest point on sl to origin0
	if( IsZero(denom) )
	{
		// clamp s_c to 0
		sd = td = c;
		sn = 0.0f;
		tn = e;
	}
	else
	{
		// clamp s_c within [0,1]
		sd = td = denom;
		sn = b*e - c*d;
		tn = a*e - b*d;

		// clamp s_c to 0
		if (sn < 0.0f)
		{
			sn = 0.0f;
			tn = e;
			td = c;
		}
		// clamp s_c to 1
		else if (sn > sd)
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	// clamp t_c within [0,1]
	// clamp t_c to 0
	if( tn < 0.0f )
	{
		t_c = 0.0f;
		// clamp s_c to 0
		if ( -d < 0.0f )
			s_c = 0.0f;
		// clamp s_c to 1
		else if ( -d > a )
			s_c = 1.0f;
		else
			s_c = -d/a;
	}
	// clamp t_c to 1
	else if( tn > td )
	{
		t_c = 1.0f;
		// clamp s_c to 0
		if( (-d+b) < 0.0f )
			s_c = 0.0f;
		// clamp s_c to 1
		else if( (-d+b) > a )
			s_c = 1.0f;
		else
			s_c = (-d+b)/a;
	}
	else
	{
			t_c = tn/td;
			s_c = sn/sd;
	}

	// compute difference vector and distance squared
	vec3_t wc = w0 + dir*s_c - ls.dir*t_c;
	return wc.Dot(wc);
}


/*
=======================================================================================================================================
DistanceSquared                                                                                                                       =
seg - ray                                                                                                                             =
=======================================================================================================================================
*/
float lineseg_t::DistanceSquared( const ray_t& ray, float& s_c, float& t_c ) const
{
	// compute intermediate parameters
	vec3_t w0 = origin - ray.origin;
	float a = dir.Dot( dir );
	float b = dir.Dot( ray.dir );
	float c = ray.dir.Dot( ray.dir );
	float d = dir.Dot( w0 );
	float e = ray.dir.Dot( w0 );

	float denom = a*c - b*b;
	// parameters to compute s_c, t_c
	float sn, sd, tn, td;

	// if denom is zero, try finding closest point on ME1 to origin0
	if( IsZero(denom) )
	{
		// clamp s_c to 0
		sd = td = c;
		sn = 0.0f;
		tn = e;
	}
	else
	{
		// clamp s_c within [0,1]
		sd = td = denom;
		sn = b*e - c*d;
		tn = a*e - b*d;

		// clamp s_c to 0
		if (sn < 0.0f)
		{
			sn = 0.0f;
			tn = e;
			td = c;
		}
		// clamp s_c to 1
		else if (sn > sd)
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	// clamp t_c within [0,+inf]
	// clamp t_c to 0
	if (tn < 0.0f)
	{
		t_c = 0.0f;
		// clamp s_c to 0
		if ( -d < 0.0f )
			s_c = 0.0f;
		// clamp s_c to 1
		else if ( -d > a )
			s_c = 1.0f;
		else
			s_c = -d/a;
	}
	else
	{
		t_c = tn/td;
		s_c = sn/sd;
	}

	// compute difference vector and distance squared
	vec3_t wc = w0 + dir*s_c - ray.dir*t_c;
	return wc.Dot(wc);
}


/*
=======================================================================================================================================
DistanceSquared                                                                                                                       =
seg - point                                                                                                                           =
=======================================================================================================================================
*/
float lineseg_t::DistanceSquared( const vec3_t& point, float& t_c ) const
{
	vec3_t w = point - origin;
	float proj = w.Dot(dir);
	// endpoint 0 is closest point
	if ( proj <= 0 )
	{
		t_c = 0.0f;
		return w.Dot(w);
	}
	else
	{
		float vsq = dir.Dot(dir);
		// endpoint 1 is closest point
		if ( proj >= vsq )
		{
			t_c = 1.0f;
			return w.Dot(w) - 2.0f*proj + vsq;
		}
		// otherwise somewhere else in segment
		else
		{
			t_c = proj/vsq;
			return w.Dot(w) - t_c*proj;
		}
	}
}


/*
=======================================================================================================================================
ClosestPoints                                                                                                                         =
seg - seg                                                                                                                             =
=======================================================================================================================================
*/
void lineseg_t::ClosestPoints( const lineseg_t& segment1, vec3_t& point0, vec3_t& point1 ) const
{
  // compute intermediate parameters
  vec3_t w0 = origin - segment1.origin;
  float a = dir.Dot( dir );
  float b = dir.Dot( segment1.dir );
  float c = segment1.dir.Dot( segment1.dir );
  float d = dir.Dot( w0 );
  float e = segment1.dir.Dot( w0 );

  float denom = a*c - b*b;
  // parameters to compute s_c, t_c
  float s_c, t_c;
  float sn, sd, tn, td;

  // if denom is zero, try finding closest point on segment1 to origin0
  if( IsZero(denom) )
  {
		// clamp s_c to 0
		sd = td = c;
		sn = 0.0f;
		tn = e;
  }
  else
  {
		// clamp s_c within [0,1]
		sd = td = denom;
		sn = b*e - c*d;
		tn = a*e - b*d;

		// clamp s_c to 0
		if(sn < 0.0f)
		{
			sn = 0.0f;
			tn = e;
			td = c;
		}
		// clamp s_c to 1
		else if(sn > sd)
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
  }

  // clamp t_c within [0,1]
  // clamp t_c to 0
  if(tn < 0.0f)
  {
		t_c = 0.0f;
		// clamp s_c to 0
		if( -d < 0.0f )
			s_c = 0.0f;
		// clamp s_c to 1
		else if( -d > a )
			s_c = 1.0f;
		else
			s_c = -d/a;
  }
  // clamp t_c to 1
  else if(tn > td)
  {
		t_c = 1.0f;
		// clamp s_c to 0
		if( (-d+b) < 0.0f )
			s_c = 0.0f;
		// clamp s_c to 1
		else if( (-d+b) > a )
			s_c = 1.0f;
		else
			s_c = (-d+b)/a;
  }
  else
  {
     t_c = tn/td;
     s_c = sn/sd;
  }

  // compute closest points
  point0 = origin + dir*s_c;
  point1 = segment1.origin + segment1.dir*t_c;

}

/*
=======================================================================================================================================
ClosestPoints                                                                                                                         =
seg - ray                                                                                                                             =
=======================================================================================================================================
*/
void lineseg_t::ClosestPoints( const ray_t& ray, vec3_t& point0, vec3_t& point1 ) const
{
	// compute intermediate parameters
	vec3_t w0 = origin - ray.origin;
	float a = dir.Dot( dir );
	float b = dir.Dot( ray.dir );
	float c = ray.dir.Dot( ray.dir );
	float d = dir.Dot( w0 );
	float e = ray.dir.Dot( w0 );

	float denom = a*c - b*b;
	// parameters to compute s_c, t_c
	float s_c, t_c;
	float sn, sd, tn, td;

	// if denom is zero, try finding closest point on 1 to origin0
	if( IsZero(denom) )
	{
		// clamp s_c to 0
		sd = td = c;
		sn = 0.0f;
		tn = e;
	}
	else
	{
		// clamp s_c within [0,1]
		sd = td = denom;
		sn = b*e - c*d;
		tn = a*e - b*d;

		// clamp s_c to 0
		if( sn < 0.0f )
		{
			sn = 0.0f;
			tn = e;
			td = c;
		}
		// clamp s_c to 1
		else if( sn > sd )
		{
			sn = sd;
			tn = e + b;
			td = c;
		}
	}

	// clamp t_c within [0,+inf]
	// clamp t_c to 0
	if( tn < 0.0f )
	{
		t_c = 0.0f;
		// clamp s_c to 0
		if( -d < 0.0f )
		{
			s_c = 0.0f;
		}
		// clamp s_c to 1
		else if( -d > a )
		{
			s_c = 1.0f;
		}
		else
		{
			s_c = -d/a;
		}
	}
	else
	{
		t_c = tn/td;
		s_c = sn/sd;
	}

	// compute closest points
	point0 = origin + dir*s_c;
	point1 = ray.origin + ray.dir*t_c;

}


/*
=======================================================================================================================================
ClosestPoints                                                                                                                         =
seg - point                                                                                                                           =
=======================================================================================================================================
*/
vec3_t lineseg_t::ClosestPoints( const vec3_t& point ) const
{
    vec3_t w = point - origin;
    float proj = w.Dot( dir );
    // endpoint 0 is closest point
    if( proj <= 0.0f )
			return origin;
    else
    {
			float vsq = dir.Dot(dir);
			// endpoint 1 is closest point
			if( proj >= vsq )
				return origin + dir;
			// else somewhere else in segment
			else
				return origin + dir*(proj/vsq);
    }
}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
lineseg_t lineseg_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale  ) const
{
	lineseg_t seg;

	seg.origin = origin.Transformed( translate, rotate, scale );
	seg.dir = rotate * (dir * scale);

	return seg;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void lineseg_t::Render()
{
	vec3_t P1 = origin+dir;

	glColor3fv( &vec3_t(1.0f, 1.0f, 1.0f )[0] );
	glBegin( GL_LINES );
		glVertex3fv( &origin[0] );
		glVertex3fv( &P1[0] );
	glEnd();


	//glPointSize( 4.0f );
	glBegin( GL_POINTS );
		glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
		glVertex3fv( &origin[0] );
		glColor3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
		glVertex3fv( &P1[0] );
	glEnd();

	glDisable( GL_DEPTH_TEST );
	glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );
	glBegin( GL_LINES );
		glVertex3fv( &origin[0] );
		glVertex3fv( &(P1)[0] );
	glEnd();
}


/*
=======================================================================================================================================
PlaneTest                                                                                                                             =
=======================================================================================================================================
*/
float lineseg_t::PlaneTest( const plane_t& plane ) const
{
	const vec3_t& P0 = origin;
	vec3_t P1 = origin+dir;

	float dist0 = plane.Test( P0 );
	float dist1 = plane.Test( P1 );

	if( dist0 > 0.0 )
	{
		if( dist1 < 0.0 )
			return 0.0;
		else
			return Min( dist0, -dist1 );
	}
	else
	{
		if( dist1 > 0.0 )
			return 0.0;
		else
			return Min( -dist0, dist1 );
	}
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
line - sphere                                                                                                                         =
=======================================================================================================================================
*/
bool lineseg_t::Intersects( const bsphere_t& sphere ) const
{
	return sphere.Intersects( *this );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
line - aabb                                                                                                                           =
=======================================================================================================================================
*/
bool lineseg_t::Intersects( const aabb_t& box ) const
{
	return box.Intersects( *this );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
line - obb                                                                                                                            =
=======================================================================================================================================
*/
bool lineseg_t::Intersects( const obb_t& obb ) const
{
	return obb.Intersects( *this );
}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
ray_t ray_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	/*ray_t ray;

	mat4_t semi_transf( rotate*scale );

	ray.dir = semi_transf * dir;
	ray.dir.Normalize();

	ray.origin = semi_transf * origin;
	ray.origin += translate;

	return ray;*/
	ray_t new_ray;

	new_ray.origin = origin.Transformed( translate, rotate, scale );
	new_ray.dir = rotate * dir;

	return new_ray;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void ray_t::Render()
{
	const float dist = 100.0f;
	glColor3fv( &vec3_t( 1.0f, 1.0f, 0.0f )[0] );

	// render a dotted without depth
	glDisable( GL_DEPTH_TEST );

	glBegin( GL_LINES );
		glVertex3fv( &origin[0] );
		glVertex3fv( &(dir*dist+origin)[0] );
	glEnd();

	//glPointSize( 4.0f );
	glBegin( GL_POINTS );
		glVertex3fv( &origin[0] );
	glEnd();

	// render with depth
	glBegin( GL_LINES );
		glVertex3fv( &origin[0] );
		glVertex3fv( &(dir*dist+origin)[0] );
	glEnd();

}


/*
=======================================================================================================================================
PlaneTest                                                                                                                             =
=======================================================================================================================================
*/
float ray_t::PlaneTest( const plane_t& plane ) const
{
	float dist = plane.Test( origin );
	float cos_ = plane.normal.Dot( dir );

	if( cos_ > 0.0 ) // the ray points to the same half-space as the plane
	{
		if( dist < 0.0 ) // the ray's origin is behind the plane
			return 0.0;
		else
			return dist;
	}
	else
	{
		if( dist > 0.0 )
			return 0.0;
		else
			return dist;
	}
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
ray - sphere                                                                                                                          =
=======================================================================================================================================
*/
bool ray_t::Intersects( const bsphere_t& sphere ) const
{
	return sphere.Intersects( *this );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
ray - aabb                                                                                                                            =
=======================================================================================================================================
*/
bool ray_t::Intersects( const aabb_t& box ) const
{
	return box.Intersects( *this );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
ray - obb                                                                                                                             =
=======================================================================================================================================
*/
bool ray_t::Intersects( const obb_t& obb ) const
{
	return obb.Intersects( *this );
}



/*
=======================================================================================================================================
Set                                                                                                                                   =
from plane eqtuation                                                                                                                  =
=======================================================================================================================================
*/
void plane_t::Set( float a, float b, float c, float d )
{
	// normalize for cheap distance checks
	float lensq = a*a + b*b + c*c;
	// length of normal had better not be zero
	DEBUG_ERR( IsZero( lensq ) );

	// recover gracefully
	if ( IsZero( lensq ) )
	{
		normal = vec3_t( 1.0, 0.0, 0.0 );
		offset = 0.0f;
	}
	else
	{
		float recip = InvSqrt(lensq);
		normal = vec3_t( a*recip, b*recip, c*recip );
		offset = d*recip;
	}
}


/*
=======================================================================================================================================
Set                                                                                                                                   =
from 3 points                                                                                                                         =
=======================================================================================================================================
*/
void plane_t::Set( const vec3_t& p0, const vec3_t& p1, const vec3_t& p2 )
{
	// get plane vectors
	vec3_t u = p1 - p0;
	vec3_t v = p2 - p0;

	normal = u.Cross(v);

	// length of normal had better not be zero
	DEBUG_ERR( IsZero( normal.LengthSquared() ) );

	normal.Normalize();
	offset = normal.Dot(p0); // ToDo: correct??

}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
plane_t plane_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	plane_t plane;

	// the normal
	plane.normal = rotate*normal;

	// the offset
	vec3_t new_trans = rotate.Transposed() * translate;
	plane.offset = offset*scale + new_trans.Dot( normal );

	return plane;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void plane_t::Render()
{
	glPushMatrix();

	vec3_t translate( normal*offset );
	quat_t q;
	q.CalcFrom2Vec3( vec3_t( 0.0, 0.0, 1.0 ), normal );
	mat3_t rotate( q );
	mat4_t transform( translate, rotate );
	r::MultMatrix( transform );

	glColor4fv( &vec4_t(1.0f, 1.0f, 1.0f, 0.5f)[0] );

	const float size = 10.0f;

	glBegin( GL_QUADS );
		glVertex3fv( &vec3_t(size, size, 0.0f)[0] );
		glVertex3fv( &vec3_t(-size, size, 0.0f)[0] );
		glVertex3fv( &vec3_t(-size, -size, 0.0f)[0] );
		glVertex3fv( &vec3_t(size, -size, 0.0f)[0] );
	glEnd();

	glColor4fv( &vec4_t(1.0f, 1.0f, 1.0f, 0.2f)[0] );
	glBegin( GL_QUADS );
		glVertex3fv( &vec3_t(size, -size, 0.0f)[0] );
		glVertex3fv( &vec3_t(-size, -size, 0.0f)[0] );
		glVertex3fv( &vec3_t(-size, size, 0.0f)[0] );
		glVertex3fv( &vec3_t(size, size, 0.0f)[0] );
	glEnd();

	glPopMatrix();

	glDisable( GL_DEPTH_TEST );
	glColor3fv( &vec3_t(1, 1, 0)[0] );
	glBegin( GL_LINES );
		glVertex3fv( &(normal*offset)[0] );
		glVertex3fv( &(normal*(offset+1))[0] );
	glEnd();
}



/*
=======================================================================================================================================
Set                                                                                                                                   =
from a vec3 array                                                                                                                     =
=======================================================================================================================================
*/
void bsphere_t::Set( const void* pointer, uint stride, int count )
{
	void* tmp_pointer = (void*)pointer;
	vec3_t min( *(vec3_t*)tmp_pointer ),
	       max( *(vec3_t*)tmp_pointer );

	// for all the vec3 calc the max and min
	for( int i=1; i<count; i++ )
	{
		tmp_pointer = (char*)tmp_pointer + stride;

		const vec3_t& tmp = *((vec3_t*)tmp_pointer);

		for( int j=0; j<3; j++ )
		{
			if( tmp[j] > max[j] )
				max[j] = tmp[j];
			else if( tmp[j] < min[j] )
				min[j] = tmp[j];
		}
	}

	center = (min+max) * 0.5; // average

	tmp_pointer = (void*)pointer;
	float max_dist = (*((vec3_t*)tmp_pointer) - center).LengthSquared(); // max distance between center and the vec3 arr
	for( int i=1; i<count; i++ )
	{
		tmp_pointer = (char*)tmp_pointer + stride;

		const vec3_t& vertco = *((vec3_t*)tmp_pointer);
		float dist = (vertco - center).LengthSquared();
		if( dist > max_dist )
			max_dist = dist;
	}

	radius = Sqrt( max_dist );
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void bsphere_t::Render()
{
	glPushMatrix();

	glTranslatef( center.x, center.y, center.z );

	glColor4fv( &vec4_t(1.0, 1.0, 1.0, 0.2)[0] );

	r::dbg::RenderSphere( radius, 24 );

	glPopMatrix();
}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
bsphere_t bsphere_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	bsphere_t new_sphere;

	new_sphere.center = center.Transformed( translate, rotate, scale );
	new_sphere.radius = radius * scale;
	return new_sphere;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
sphere - sphere                                                                                                                       =
=======================================================================================================================================
*/
bool bsphere_t::Intersects( const bsphere_t& other ) const
{
	float tmp = radius + other.radius;
	return (center-other.center).LengthSquared() <= tmp*tmp ;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
sphere - aabb                                                                                                                         =
=======================================================================================================================================
*/
bool bsphere_t::Intersects( const aabb_t& box ) const
{
	return box.Intersects(*this);
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
sphere - ray                                                                                                                          =
=======================================================================================================================================
*/
bool bsphere_t::Intersects( const ray_t& ray ) const
{
	vec3_t w( center - ray.origin );
	const vec3_t& v = ray.dir;
	float proj = v.Dot( w );
	float wsq = w.LengthSquared();
	float rsq = radius*radius;

	if( proj < 0.0 && wsq > rsq )
		return false;

	float vsq = v.LengthSquared();

	return (vsq*wsq - proj*proj <= vsq*rsq);
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
sphere - segment                                                                                                                      =
=======================================================================================================================================
*/
bool bsphere_t::Intersects( const lineseg_t& segment ) const
{
	const vec3_t& v = segment.dir;
	vec3_t w0 = center - segment.origin;
	float w0dv = w0.Dot( v );
	float rsq = radius * radius;

	if( w0dv < 0.0f ) // if the ang is >90
		return w0.LengthSquared() <= rsq;

	vec3_t w1 = w0 - v; // aka center - P1, where P1 = seg.origin + seg.dir
	float w1dv = w1.Dot( v );

	if( w1dv > 0.0f ) // if the ang is <90
		return w1.LengthSquared() <= rsq;

	vec3_t tmp = w0 - ( v * (w0.Dot(v) / v.LengthSquared()) ); // the big parenthesis is the projection of w0 to v
	return tmp.LengthSquared() <= rsq;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
sphere - obb                                                                                                                          =
=======================================================================================================================================
*/
bool bsphere_t::Intersects( const obb_t& obb ) const
{
	return obb.Intersects( *this );
}


/*
=======================================================================================================================================
PlaneTest                                                                                                                             =
=======================================================================================================================================
*/
float bsphere_t::PlaneTest( const plane_t& plane ) const
{
	float dist = plane.Test( center );

	if( dist > radius )
		return dist-radius;
	else if( -dist > radius )
		return dist+radius;
	else
		return 0.0f;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
sphere - sphere                                                                                                                       =
=======================================================================================================================================
*/
bool bsphere_t::SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	normal = sphere.center - center;
	float rsum = radius + sphere.radius;
	float distsq = normal.LengthSquared();

	if( distsq <= rsum*rsum )
	{
		// calc the depth
		float dist = Sqrt( distsq );
		depth = rsum - dist;

		normal.Normalize();

		impact_point = ((center + normal*radius) + (sphere.center - normal*sphere.radius)) * 0.5f;

		RENDER_SEPERATION_TEST

		return true;
	}

	return false;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
sphere - aabb                                                                                                                         =
=======================================================================================================================================
*/
bool bsphere_t::SeperationTest( const aabb_t& box, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	UNLOCK_RENDER_SEPERATION
	bool test = box.SeperationTest( *this, normal, impact_point, depth );
	LOCK_RENDER_SEPERATION
	impact_point = impact_point + (normal*depth);
	normal = -normal;

	if( test ) RENDER_SEPERATION_TEST;

	return test;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
sphere - obb                                                                                                                          =
=======================================================================================================================================
*/
bool bsphere_t::SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	UNLOCK_RENDER_SEPERATION
	bool test = obb.SeperationTest( *this, normal, impact_point, depth );
	LOCK_RENDER_SEPERATION

	if( !test ) return false;

	impact_point = impact_point + (normal*depth);
	normal = -normal;

	RENDER_SEPERATION_TEST;
	return true;
}


/*
=======================================================================================================================================
Set                                                                                                                                   =
Calc origin and radius from a vec3 array                                                                                              =
=======================================================================================================================================
*/
void aabb_t::Set( const void* pointer, uint stride, int count )
{
	void* tmp_pointer = (void*)pointer;
	min = *(vec3_t*)tmp_pointer;
	max = *(vec3_t*)tmp_pointer;

	// for all the vec3 calc the max and min
	for( int i=1; i<count; i++ )
	{
		tmp_pointer = (char*)tmp_pointer + stride;

		vec3_t tmp( *(vec3_t*)tmp_pointer );

		for( int j=0; j<3; j++ )
		{
			if( tmp[j] > max[j] )
				max[j] = tmp[j];
			else if( tmp[j] < min[j] )
				min[j] = tmp[j];
		}
	}
}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
aabb_t aabb_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	/*aabb_t aabb;
	aabb.min = min * scale;
	aabb.max = max * scale;

	aabb.min += translate;
	aabb.max += translate;
	return aabb;*/
	aabb_t new_aabb;

	// if there is no rotation our job is easy
	if( rotate == mat3_t::GetIdentity() )
	{
		new_aabb.min = (min * scale) + translate;
		new_aabb.max = (max * scale) + translate;
	}
	// if not then we are fucked
	else
	{
		vec3_t points [8] = { max, vec3_t(min.x,max.y,max.z), vec3_t(min.x,min.y,max.z), vec3_t(max.x,min.y,max.z),
		                      vec3_t(max.x,max.y,min.z), vec3_t(min.x,max.y,min.z), min, vec3_t(max.x,min.y,min.z) };

		for( int i=0; i<8; i++ )
			points[i].Transform( translate, rotate, scale );

		new_aabb.Set( points, 0, 8 );
	}

	return new_aabb;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void aabb_t::Render()
{
	glPushMatrix();

	vec3_t sub( max-min );
	vec3_t center( (max+min)*0.5 );

	glTranslatef( center.x, center.y, center.z );
	glScalef( sub.x, sub.y, sub.z );

	glColor3fv( &vec3_t( 1.0, 1.0, 1.0 )[0] );

	r::dbg::RenderCube();

	glPopMatrix();

	glBegin( GL_POINTS );
		glColor3fv( &vec3_t( 1.0, 0.0, 0.0 )[0] );
		glVertex3fv( &min[0] );
		glColor3fv( &vec3_t( 0.0, 1.0, 0.0 )[0] );
		glVertex3fv( &max[0] );
	glEnd();
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
aabb - aabb                                                                                                                           =
=======================================================================================================================================
*/
bool aabb_t::Intersects( const aabb_t& other ) const
{
	// if separated in x direction
	if( min.x > other.max.x || other.min.x > max.x )
		return false;

	// if separated in y direction
	if( min.y > other.max.y || other.min.y > max.y )
		return false;

	// if separated in z direction
	if( min.z > other.max.z || other.min.z > max.z )
		return false;

	// no separation, must be intersecting
	return true;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
aabb - sphere                                                                                                                         =
=======================================================================================================================================
*/
bool aabb_t::Intersects( const bsphere_t& sphere ) const
{
	const vec3_t& c = sphere.center;

	// find the box's closest point to the sphere
	vec3_t cp; // Closest Point
	for( uint i=0; i<3; i++ )
	{
		if( c[i] > max[i] ) // if the center is greater than the max then the closest point is the max
			cp[i] = max[i];
		else if( c[i] < min[i]  ) // relative to the above
			cp[i] = min[i];
		else           // the c lies between min and max
			cp[i] = c[i];
	}

	float rsq = sphere.radius * sphere.radius;
	vec3_t sub = c - cp; // if the c lies totaly inside the box then the sub is the zero,
	                     //this means that the length is also zero and thus its always smaller than rsq

	if( sub.LengthSquared() <= rsq ) return true;

	return false;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
aabb - ray                                                                                                                            =
=======================================================================================================================================
*/
bool aabb_t::Intersects( const ray_t& ray ) const
{
	float maxS = -FLT_MAX;
	float minT = FLT_MAX;

	// do tests against three sets of planes
	for ( int i = 0; i < 3; ++i )
	{
		// ray is parallel to plane
		if ( IsZero( ray.dir[i] ) )
		{
			// ray passes by box
			if ( ray.origin[i] < min[i] || ray.origin[i] > max[i] )
				return false;
		}
		else
		{
			// compute intersection parameters and sort
			float s = (min[i] - ray.origin[i])/ray.dir[i];
			float t = (max[i] - ray.origin[i])/ray.dir[i];
			if ( s > t )
			{
				float temp = s;
				s = t;
				t = temp;
			}

			// adjust min and max values
			if ( s > maxS )
				maxS = s;
			if ( t < minT )
				minT = t;
			// check for intersection failure
			if ( minT < 0.0f || maxS > minT )
				return false;
		}
	}

	// done, have intersection
	return true;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
aabb - segment                                                                                                                        =
=======================================================================================================================================
*/
bool aabb_t::Intersects( const lineseg_t& segment ) const
{
	float maxS = -FLT_MAX;
	float minT = FLT_MAX;

	// do tests against three sets of planes
	for( int i = 0; i < 3; ++i )
	{
		// segment is parallel to plane
		if( IsZero( segment.dir[i] ) )
		{
			// segment passes by box
			if( (segment.origin)[i] < min[i] || (segment.origin)[i] > max[i] )
				return false;
		}
		else
		{
			// compute intersection parameters and sort
			float s = (min[i] - segment.origin[i])/segment.dir[i];
			float t = (max[i] - segment.origin[i])/segment.dir[i];
			if( s > t )
			{
				float temp = s;
				s = t;
				t = temp;
			}

			// adjust min and max values
			if( s > maxS )
				maxS = s;
			if( t < minT )
				minT = t;
			// check for intersection failure
			if( minT < 0.0f || maxS > 1.0f || maxS > minT )
				return false;
		}
	}

	// done, have intersection
	return true;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
aabb - obb                                                                                                                            =
=======================================================================================================================================
*/
bool aabb_t::Intersects( const obb_t& obb ) const
{
	return obb.Intersects(*this);
}


/*
=======================================================================================================================================
PlaneTest                                                                                                                             =
=======================================================================================================================================
*/
float aabb_t::PlaneTest( const plane_t& plane ) const
{
	vec3_t diag_min, diag_max;
	// set min/max values for x,y,z direction
	for( int i=0; i<3; i++ )
	{
		if( plane.normal[i] >= 0.0f )
		{
			diag_min[i] = min[i];
			diag_max[i] = max[i];
		}
		else
		{
			diag_min[i] = max[i];
			diag_max[i] = min[i];
		}
	}

	// minimum on positive side of plane, box on positive side
	float test = plane.Test( diag_min );
	if ( test > 0.0f )
		return test;

	test = plane.Test( diag_max );
	// min on non-positive side, max on non-negative side, intersection
	if ( test >= 0.0f )
		return 0.0f;
	// max on negative side, box on negative side
	else
		return test;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
aabb - aabb                                                                                                                           =
=======================================================================================================================================
*/
bool aabb_t::SeperationTest( const aabb_t& other, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	// calculete the closest points
	for( uint i=0; i<3; i++ ) // for 3 axis
	{
		if( min[i] > other.max[i] || other.min[i] > max[i] )
			return false;

		const float& Am = min[i], AM = max[i], Bm = other.min[i], BM = other.max[i];

		if( Bm < Am )
		{
			if( BM < Am ) // B is left and outside A
				return false;
			else
				if( BM < AM ) // left
				{
					normal[i] = Am - BM;
					impact_point[i] = (Am+BM) * 0.5f;
				}
				else // B overlaps A
				{
					float t0 = AM-Bm, t1 = BM-Am;
					if( t0 < t1 )
						normal[i] = t0;
					else
						normal[i] = -t1;

					impact_point[i] = (Am+AM) * 0.5f;
				}
		}
		else
		{
			if( Bm > AM ) // B is right and outside A
				return false;
			else
				if( BM < AM ) // B totaly inside A
				{
					float t0 = BM-Am, t1 = AM-Bm;
					if( t0 < t1 )
						normal[i] = -t0;
					else
						normal[i] = t1;

					impact_point[i] = (Bm+BM) * 0.5f;
				}
				else // right
				{
					normal[i] = AM - Bm;
					impact_point[i] = (AM + Bm) * 0.5f;
				}
		}

	}

	vec3_t dist( fabs(normal.x), fabs(normal.y), fabs(normal.z) );
	if( dist.x < dist.y && dist.x < dist.z )
		normal = vec3_t( normal.x, 0.0f, 0.0f );
	else if( dist.y < dist.z )
		normal = vec3_t( 0.0f, normal.y, 0.0f );
	else
		normal = vec3_t( 0.0f, 0.0f, normal.z );

	depth = normal.Length();

	normal *= 1.0f/depth; // aka normal.Normalize()

	RENDER_SEPERATION_TEST

	return true;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
aabb - obb                                                                                                                           =
=======================================================================================================================================
*/
bool aabb_t::SeperationTest( const obb_t& obb, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	UNLOCK_RENDER_SEPERATION
	bool test = obb.SeperationTest( *this, normal, impact_point, depth );
	LOCK_RENDER_SEPERATION

	if( !test ) return false;

	impact_point = impact_point + (normal*depth);
	normal = -normal;

	RENDER_SEPERATION_TEST;
	return true;
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
aabb - sphere                                                                                                                         =
=======================================================================================================================================
*/
bool aabb_t::SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	const vec3_t& c = sphere.center;
	const float r = sphere.radius;
	vec3_t cp; // closest point of box that its closer to the sphere's center

	for( int i=0; i<3; i++ )
	{
		if( c[i] >= max[i] ) // if the center is greater than the max then the closest point is the max
			cp[i] = max[i];
		else if( c[i] <= min[i]  ) // relative to the above
			cp[i] = min[i];
		else           // the c lies between min and max
			cp[i] = c[i];
	}

	vec3_t sub = c - cp; // if the c lies totaly inside the box then the sub is the zero,
	                     // this means that the length is also zero and thus its always smaller than rsq

	float sublsq = sub.LengthSquared();
	if( sublsq > r*r ) return false; // if no collision leave before its too late

	if( IsZero(sublsq) ) // this means that the closest point is coincide with the center so the center is totaly inside tha box. We have to revise the calcs
	{
		int n_axis = 0; // the axis that the normal will be
		float min_d = FLT_MAX; // in the end of "for" the min_d holds the min projection dist of c to every cube's facet
		float coord = 0.0;
		for( int i=0; i<3; i++ )
		{
			// dist between c and max/min in the i axis
			float dist_c_max = max[i]-c[i];
			float dist_c_min = c[i]-min[i];

			if( dist_c_max < min_d && dist_c_max < dist_c_min )
			{
				min_d = dist_c_max;
				n_axis = i;
				coord = max[i];
			}
			else if( dist_c_min < min_d )
			{
				min_d = dist_c_min;
				n_axis = i;
				coord = min[i];
			}
		}

		float dif = coord - c[n_axis];

		normal = vec3_t( 0.0, 0.0, 0.0 );
		normal[n_axis] = dif / min_d; // aka ... = (dif<0.0f) ? -1.0f : 1.0f;

		depth = r + min_d;

		impact_point = c-(normal*r);
	}
	// the c is outside the box
	else
	{
		normal = c - cp;

		depth = r - normal.Length();

		normal.Normalize();

		impact_point = c-(normal*r);
	}

	RENDER_SEPERATION_TEST

	return true;
}


/*
=======================================================================================================================================
Set                                                                                                                                   =
calc from a vec3 array                                                                                                                =
=======================================================================================================================================
*/
void obb_t::Set( const void* pointer, uint stride, int count )
{
	void* tmp_pointer = (void*)pointer;
	vec3_t min = *(vec3_t*)tmp_pointer;
	vec3_t max = *(vec3_t*)tmp_pointer;

	// for all the vec3 calc the max and min
	for( int i=1; i<count; i++ )
	{
		tmp_pointer = (char*)tmp_pointer + stride;

		vec3_t tmp( *(vec3_t*)tmp_pointer );

		for( int j=0; j<3; j++ ) // for x y z
		{
			if( tmp[j] > max[j] )
				max[j] = tmp[j];
			else if( tmp[j] < min[j] )
				min[j] = tmp[j];
		}
	}

	// set the locals
	center = (max+min)*0.5f;
	rotation = mat3_t::GetIdentity();
	extends = max-center;
}


/*
=======================================================================================================================================
Render                                                                                                                                =
=======================================================================================================================================
*/
void obb_t::Render()
{
	glPushMatrix();

	glTranslatef( center.x, center.y, center.z ); // translate
	r::MultMatrix( mat4_t(rotation) ); // rotate
	glScalef( extends.x, extends.y, extends.z ); // scale

	glColor3fv( &vec3_t(1.0f, 1.0f, 1.0f)[0] );

	r::dbg::RenderCube( false, 2.0f );

	r::Color3( vec3_t( 0.0, 1.0, 0.0 ) );
	glBegin( GL_POINTS );
		glVertex3fv( &vec3_t(1.0, 1.0, 1.0)[0] );
	glEnd();

	glPopMatrix();
}


/*
=======================================================================================================================================
Transformed                                                                                                                           =
=======================================================================================================================================
*/
obb_t obb_t::Transformed( const vec3_t& translate, const mat3_t& rotate, float scale ) const
{
	obb_t res;

	res.extends = extends * scale;
	res.center = center.Transformed( translate, rotate, scale );
	res.rotation = rotate * rotation;

	return res;
}


/*
=======================================================================================================================================
PlaneTest                                                                                                                             =
=======================================================================================================================================
*/
float obb_t::PlaneTest( const plane_t& plane ) const
{
	vec3_t x_normal = rotation.Transposed() * plane.normal;
	// maximum extent in direction of plane normal
	float r = fabs(extends.x*x_normal.x)
					+ fabs(extends.y*x_normal.y)
					+ fabs(extends.z*x_normal.z);
	// signed distance between box center and plane
	float d = plane.Test(center);

	// return signed distance
	if( fabs(d) < r )
		return 0.0f;
	else if( d < 0.0f )
		return d + r;
	else
		return d - r;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
obb - obb                                                                                                                             =
=======================================================================================================================================
*/
bool obb_t::Intersects( const obb_t& other ) const
{
	// extent vectors
	const vec3_t& a = extends;
	const vec3_t& b = other.extends;

	// test factors
	float cTest, aTest, bTest;
	bool parallelAxes = false;

	// transpose of rotation of B relative to A, i.e. (R_b^T * R_a)^T
	mat3_t Rt = rotation.Transposed() * other.rotation;

	// absolute value of relative rotation matrix
	mat3_t Rabs;
	for( uint i = 0; i < 3; ++i )
	{
		for( uint j = 0; j < 3; ++j )
		{
			Rabs(i,j) = fabs( Rt(i,j ) );
			// if magnitude of dot product between axes is close to one
			if ( Rabs(i,j) + EPSILON >= 1.0f )
			{
				// then box A and box B have near-parallel axes
				parallelAxes = true;
			}
		}
	}

	// relative translation (in A's frame)
	vec3_t c = rotation.Transposed()*(other.center - center);

	// separating axis A0
	cTest = fabs(c.x);
	aTest = a.x;
	bTest = b.x*Rabs(0,0)+b.y*Rabs(0,1)+b.z*Rabs(0,2);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A1
	cTest = fabs(c.y);
	aTest = a.y;
	bTest = b.x*Rabs(1,0)+b.y*Rabs(1,1)+b.z*Rabs(1,2);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A2
	cTest = fabs(c.z);
	aTest = a.z;
	bTest = b.x*Rabs(2,0)+b.y*Rabs(2,1)+b.z*Rabs(2,2);
	if ( cTest > aTest + bTest ) return false;

	// separating axis B0
	cTest = fabs( c.x*Rt(0,0) + c.y*Rt(1,0) + c.z*Rt(2,0) );
	aTest = a.x*Rabs(0,0)+a.y*Rabs(1,0)+a.z*Rabs(2,0);
	bTest = b.x;
	if ( cTest > aTest + bTest ) return false;

	// separating axis B1
	cTest = fabs( c.x*Rt(0,1) + c.y*Rt(1,1) + c.z*Rt(2,1) );
	aTest = a.x*Rabs(0,1)+a.y*Rabs(1,1)+a.z*Rabs(2,1);
	bTest = b.y;
	if ( cTest > aTest + bTest ) return false;

	// separating axis B2
	cTest = fabs( c.x*Rt(0,2) + c.y*Rt(1,2) + c.z*Rt(2,2) );
	aTest = a.x*Rabs(0,2)+a.y*Rabs(1,2)+a.z*Rabs(2,2);
	bTest = b.z;
	if ( cTest > aTest + bTest ) return false;

	// if the two boxes have parallel axes, we're done, intersection
	if ( parallelAxes ) return true;

	// separating axis A0 x B0
	cTest = fabs(c.z*Rt(1,0)-c.y*Rt(2,0));
	aTest = a.y*Rabs(2,0) + a.z*Rabs(1,0);
	bTest = b.y*Rabs(0,2) + b.z*Rabs(0,1);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A0 x B1
	cTest = fabs(c.z*Rt(1,1)-c.y*Rt(2,1));
	aTest = a.y*Rabs(2,1) + a.z*Rabs(1,1);
	bTest = b.x*Rabs(0,2) + b.z*Rabs(0,0);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A0 x B2
	cTest = fabs(c.z*Rt(1,2)-c.y*Rt(2,2));
	aTest = a.y*Rabs(2,2) + a.z*Rabs(1,2);
	bTest = b.x*Rabs(0,1) + b.y*Rabs(0,0);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A1 x B0
	cTest = fabs(c.x*Rt(2,0)-c.z*Rt(0,0));
	aTest = a.x*Rabs(2,0) + a.z*Rabs(0,0);
	bTest = b.y*Rabs(1,2) + b.z*Rabs(1,1);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A1 x B1
	cTest = fabs(c.x*Rt(2,1)-c.z*Rt(0,1));
	aTest = a.x*Rabs(2,1) + a.z*Rabs(0,1);
	bTest = b.x*Rabs(1,2) + b.z*Rabs(1,0);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A1 x B2
	cTest = fabs(c.x*Rt(2,2)-c.z*Rt(0,2));
	aTest = a.x*Rabs(2,2) + a.z*Rabs(0,2);
	bTest = b.x*Rabs(1,1) + b.y*Rabs(1,0);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A2 x B0
	cTest = fabs(c.y*Rt(0,0)-c.x*Rt(1,0));
	aTest = a.x*Rabs(1,0) + a.y*Rabs(0,0);
	bTest = b.y*Rabs(2,2) + b.z*Rabs(2,1);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A2 x B1
	cTest = fabs(c.y*Rt(0,1)-c.x*Rt(1,1));
	aTest = a.x*Rabs(1,1) + a.y*Rabs(0,1);
	bTest = b.x*Rabs(2,2) + b.z*Rabs(2,0);
	if ( cTest > aTest + bTest ) return false;

	// separating axis A2 x B2
	cTest = fabs(c.y*Rt(0,2)-c.x*Rt(1,2));
	aTest = a.x*Rabs(1,2) + a.y*Rabs(0,2);
	bTest = b.x*Rabs(2,1) + b.y*Rabs(2,0);
	if ( cTest > aTest + bTest ) return false;

	// all tests failed, have intersection
	return true;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
obb - ray                                                                                                                             =
=======================================================================================================================================
*/
bool obb_t::Intersects( const ray_t& ray ) const
{
	aabb_t aabb_( -extends, extends );
	ray_t newray;
	mat3_t rottrans = rotation.Transposed();

	newray.origin = rottrans * ( ray.origin - center );
	newray.dir = rottrans * ray.dir;

	return aabb_.Intersects( newray );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
obb - segment                                                                                                                         =
ToDo: not working good                                                                                                                =
=======================================================================================================================================
*/
bool obb_t::Intersects( const lineseg_t& segment ) const
{
	float maxS = -FLT_MAX;
	float minT = FLT_MAX;

	// compute difference vector
	vec3_t diff = center - segment.origin;

	// for each axis do
	for( int i = 0; i < 3; ++i )
	{
		// get axis i
		vec3_t axis = rotation.GetColumn( i );

		// project relative vector onto axis
		float e = axis.Dot( diff );
		float f = segment.dir.Dot( axis );

		// ray is parallel to plane
		if( IsZero( f ) )
		{
			// ray passes by box
			if( -e - extends[i] > 0.0f || -e + extends[i] > 0.0f )
				return false;
			continue;
		}

		float s = (e - extends[i])/f;
		float t = (e + extends[i])/f;

		// fix order
		if( s > t )
		{
			float temp = s;
			s = t;
			t = temp;
		}

		// adjust min and max values
		if( s > maxS )
			maxS = s;
		if( t < minT )
			minT = t;

		// check for intersection failure
		if( minT < 0.0f || maxS > 1.0f || maxS > minT )
			return false;
	}

	// done, have intersection
	return true;
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
obb - sphere                                                                                                                          =
=======================================================================================================================================
*/
bool obb_t::Intersects( const bsphere_t& sphere ) const
{
	aabb_t aabb_( -extends, extends ); // aabb_ is in "this" frame
	vec3_t new_center = rotation.Transposed() * (sphere.center - center);
	bsphere_t sphere_( new_center, sphere.radius ); // sphere1 to "this" fame

	return aabb_.Intersects( sphere_ );
}


/*
=======================================================================================================================================
Intersects                                                                                                                            =
obb - aabb                                                                                                                            =
=======================================================================================================================================
*/
bool obb_t::Intersects( const aabb_t& aabb ) const
{
	vec3_t center_ = (aabb.max + aabb.min) * 0.5f;
	vec3_t extends_ = (aabb.max - aabb.min) * 0.5f;
	obb_t obb_( center_, mat3_t::GetIdentity(), extends_ );

	return Intersects( obb_ );
}


/*
=======================================================================================================================================
SeperationTest                                                                                                                        =
obb - sphere                                                                                                                          =
=======================================================================================================================================
*/
bool obb_t::SeperationTest( const bsphere_t& sphere, vec3_t& normal, vec3_t& impact_point, float& depth ) const
{
	aabb_t aabb_( -extends, extends ); // aabb_ is in "this" frame
	vec3_t new_center = rotation.Transposed() * (sphere.center - center); // sphere's new center
	bsphere_t sphere_( new_center, sphere.radius ); // sphere_ to "this" frame

	UNLOCK_RENDER_SEPERATION
	bool test = aabb_.SeperationTest( sphere_, normal, impact_point, depth );
	LOCK_RENDER_SEPERATION

	if( !test ) return false;

	impact_point = (rotation*impact_point) + center;
	normal = rotation * normal;

	RENDER_SEPERATION_TEST

	return true;
}






