// Copyright (C) 2014, Panagiotis Christopoulos Charitos.
// All rights reserved.
// Code licensed under the BSD License.
// http://www.anki3d.org/LICENSE

#ifndef ANKI_RESOURCE_DUMMY_RSRC_H
#define ANKI_RESOURCE_DUMMY_RSRC_H


namespace anki {


/// A dummy resource for the unit tests of the ResourceManager
class DummyRsrc
{
public:
	DummyRsrc()
	{
		++mem;
		loaded = false;
	}

	~DummyRsrc()
	{
		--mem;
		if(loaded)
		{
			--mem;
		}
	}

	void load(const char* filename)
	{
		++mem;
		loaded = true;
	}

	static int getMem()
	{
		return mem;
	}

private:
	static int mem;
	bool loaded;
};


} // end namespace


#endif
