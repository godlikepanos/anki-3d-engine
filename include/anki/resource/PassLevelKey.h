#ifndef ANKI_RESOURCE_PASS_LEVEL_KEY_H
#define ANKI_RESOURCE_PASS_LEVEL_KEY_H

#include <boost/unordered_map.hpp>


namespace anki {


/// XXX
struct PassLevelKey
{
	uint pass;
	uint level;

	PassLevelKey()
		: pass(0), level(0)
	{}

	PassLevelKey(uint pass_, uint level_)
		: pass(pass_), level(level_)
	{}
};


/// Create hash functor
struct PassLevelKeyCreateHash
{
	size_t operator()(const PassLevelKey& b) const
	{
		return b.pass * 1000 + b.level;
	}
};


/// Values comparisons functor
struct PassLevelKeyComparision
{
	bool operator()(const PassLevelKey& a,
		const PassLevelKey& b) const
	{
		return a.pass == b.pass && a.level == b.level;
	}
};


/// XXX
template<typename T>
struct PassLevelHashMap
{
	typedef boost::unordered_map<PassLevelKey, T,
		PassLevelKeyCreateHash, PassLevelKeyComparision> Type;
};


} // end namespace


#endif
