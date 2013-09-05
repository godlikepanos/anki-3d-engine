#ifndef ANKI_MISC_CONFIG_SET_H
#define ANKI_MISC_CONFIG_SET_H

#include "anki/util/StdTypes.h"
#include <unordered_map>

namespace anki {

/// A storage of configuration variables
class ConfigSet
{
public:
	/// Find an option and return it's value
	F64 get(const char* name) const
	{
		std::unordered_map<std::string, F64>::const_iterator it = 
			map.find(name);
		if(it != map.end())
		{
			return it->second;
		}
		else
		{
			throw ANKI_EXCEPTION("Option not found");
		}
	}

	/// Find an option and return it's value
	F64& get(const char* name)
	{
		std::unordered_map<std::string, F64>::iterator it = map.find(name);
		if(it != map.end())
		{
			return it->second;
		}
		else
		{
			throw ANKI_EXCEPTION("Option not found");
		}
	}

	// Set an option
	void set(const char* name, F64 value)
	{
		std::unordered_map<std::string, F64>::iterator it = map.find(name);
		if(it != map.end())
		{
			it->second = value;
		}
		else
		{
			throw ANKI_EXCEPTION("Option not found");
		}
	}

protected:
	/// Add a new option
	void newOption(const char* name, F64 value)
	{
		map[name] = value;
	}

private:
	std::unordered_map<std::string, F64> map;
};

} // end namespace anki

#endif
