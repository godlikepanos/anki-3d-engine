#ifndef SINGLETON_H
#define SINGLETON_H

#ifndef NULL
#	define NULL 0
#endif


/// This template makes a class singleton
template<typename Type>
class Singleton
{
	public:
		static Type& getInstance() {return *(instance ? instance : (instance = new Type));}

	protected:
		Singleton();
		~Singleton();

	private:
		static Type* instance;
		Singleton(Singleton const&);
		Singleton& operator=(const Singleton&);
};


template <typename Type>
Type* Singleton<Type>::instance = NULL;


#endif
