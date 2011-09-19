#ifndef DUMMY_RSRC_H
#define DUMMY_RSRC_H


/// A dummy resource for the unit tests of the ResourceManager
class DummyRsrc
{
	public:
		DummyRsrc();
		~DummyRsrc();

		void load(const char* filename);

		static int getMem()
		{
			return mem;
		}
	private:
		static int mem;
		bool loaded;
};


inline DummyRsrc::DummyRsrc()
{
	++mem;
	loaded = false;
}


inline DummyRsrc::~DummyRsrc()
{
	--mem;
	if(loaded)
	{
		--mem;
	}
}


inline void DummyRsrc::load(const char* /*filename*/)
{
	++mem;
	loaded = true;
}


#endif
