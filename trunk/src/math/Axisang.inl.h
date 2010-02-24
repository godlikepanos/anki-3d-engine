#include "m_dflt_header.h"

namespace m {


// constructor []
inline Axisang::Axisang()
	: ang(0.0), axis()
{}

// constructor [Axisang]
inline Axisang::Axisang( const Axisang& b )
	: ang(b.ang), axis(b.axis)
{}

// constructor [float, axis]
inline Axisang::Axisang( float rad, const vec3_t& axis_ )
	: ang(rad), axis(axis_)
{}

// constructor [quat]
inline Axisang::Axisang( const quat_t& q )
{
	ang = 2.0*acos( q.w );
	float length = sqrt( 1.0 - q.w*q.w );
	if( IsZero(length) )
		axis = vec3_t(0.0);
	else
	{
		length = 1.0/length;
		axis = vec3_t( q.x*length, q.y*length, q.z*length );
	}
}

// constructor [mat3]
inline Axisang::Axisang( const mat3_t& m3 )
{
	if( (fabs(m3(0,1)-m3(1,0))< EPSILON)  && (fabs(m3(0,2)-m3(2,0))< EPSILON)  && (fabs(m3(1,2)-m3(2,1))< EPSILON) )
	{

		if( (fabs(m3(0,1)+m3(1,0)) < 0.1 ) && (fabs(m3(0,2)+m3(2,0)) < 0.1) && (fabs(m3(1,2)+m3(2,1)) < 0.1) && (fabs(m3(0,0)+m3(1,1)+m3(2,2))-3) < 0.1 )
		{
			axis = vec3_t( 1.0, 0.0, 0.0 );
			ang = 0.0;
			return;
		}

		ang = PI;
		axis.x = (m3(0,0)+1)/2;
		if( axis.x > 0.0 )
			axis.x = sqrt(axis.x);
		else
			axis.x = 0;
		axis.y = (m3(1,1)+1)/2;
		if( axis.y > 0 )
			axis.y = sqrt(axis.y);
		else
			axis.y = 0;
		axis.z = (m3(2,2)+1)/2;
		if( axis.z > 0 )
			axis.z = sqrt(axis.z);
		else
			axis.z = 0.0;

		bool xZero = ( fabs(axis.x)<EPSILON );
		bool yZero = ( fabs(axis.y)<EPSILON );
		bool zZero = ( fabs(axis.z)<EPSILON );
		bool xyPositive = ( m3(0,1) > 0 );
		bool xzPositive = ( m3(0,2) > 0 );
		bool yzPositive = ( m3(1,2) > 0 );
		if( xZero && !yZero && !zZero ){
			if( !yzPositive ) axis.y = -axis.y;
		}else if( yZero && !zZero ){
			if( !xzPositive ) axis.z = -axis.z;
		}else if (zZero){
			if( !xyPositive ) axis.x = -axis.x;
		}

		return;
	}

	float s = sqrt((m3(2,1) - m3(1,2))*(m3(2,1) - m3(1,2))+(m3(0,2) - m3(2,0))*(m3(0,2) - m3(2,0))+(m3(1,0) - m3(0,1))*(m3(1,0) - m3(0,1)));

	if( fabs(s) < 0.001 ) s = 1;

	ang = acos( ( m3(0,0) + m3(1,1) + m3(2,2) - 1)/2 );
	axis.x= (m3(2,1) - m3(1,2))/s;
	axis.y= (m3(0,2) - m3(2,0))/s;
	axis.z= (m3(1,0) - m3(0,1))/s;
}


} // end namaspace
