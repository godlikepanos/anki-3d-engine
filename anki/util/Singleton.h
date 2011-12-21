#ifndef ANKI_UTIL_SINGLETON_H
#define ANKI_UTIL_SINGLETON_H

#ifndef NULL
#	define NULL 0
#endif


namespace anki {


/// This template makes a class singleton
template<typename T>
class Singleton
{
public:
	typedef T Value;

	static Value& get()
	{
		return *(instance ? instance : (instance = new Value));
	}

protected:
	Singleton();
	~Singleton();

private:
	static Value* instance;

	Singleton(const Singleton&);
	Singleton& operator=(const Singleton&);
};


template <typename T>
typename Singleton<T>::Value* Singleton<T>::instance = NULL;


} // end namespace


#endif
