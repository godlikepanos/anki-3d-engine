#include <boost/foreach.hpp>
#include <boost/range/iterator_range.hpp>


//==============================================================================
// set                                                                         =
//==============================================================================
template<typename Container>
void Obb::set(const Container& container)
{
	ASSERT(container.size() >= 1);

	Vec3 min(container.front());
	Vec3 max(container.front());

	// for all the Vec3s calc the max and min
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

	// set the locals
	center = (max + min) / 2.0;
	rotation = Mat3::getIdentity();
	extends = max - center;
}
