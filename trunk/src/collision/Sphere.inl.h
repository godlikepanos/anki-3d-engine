#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>
#include "util/Assert.h"


//==============================================================================
// set                                                                         =
//==============================================================================
template<typename Container>
void Sphere::set(const Container& container)
{
	ASSERT(container.size() >= 1);

	Vec3 min(container.front());
	Vec3 max(container.front());

	// for all the Vec3 calc the max and min
	BOOST_FOREACH(const Vec3& v,
		boost::make_iterator_range(container.begin() + 1, container.end()))
	{
		for(int j = 0; j < 3; j++)
		{
			if(v[j] > max[j])
			{
				max[j] = v[j];
			}
			else if(v[j] < min[j])
			{
				min[j] = v[j];
			}
		}
	}

	center = (min + max) * 0.5; // average

	// max distance between center and the vec3 arr
	float maxDist = (container.front() - center).getLengthSquared();

	BOOST_FOREACH(const Vec3& v,
		boost::make_iterator_range(container.begin() + 1, container.end()))
	{
		float dist = (v - center).getLengthSquared();
		if(dist > maxDist)
		{
			maxDist = dist;
		}
	}

	radius = Math::sqrt(maxDist);
}
