#include "tests/framework/Framework.h"
#include "anki/util/Object.h"

using namespace anki;

#if 0

template<typename T, typename Alloc>
struct Deleter
{
	void onChildRemoved(T* x, T* parent)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;

		Alloc alloc = x->getAllocator();

		alloc.destroy(x);
		alloc.deallocate(x, 1);
	}

	void onChildAdded(T*, T*)
	{}
};

struct Foo2: public Object<Foo2, Allocator<Foo2>,
	Deleter<Foo2, Allocator<Foo2>>>
{
	typedef Object<Foo2, Allocator<Foo2>,
		Deleter<Foo2, Allocator<Foo2>>> Base;

	int x = 666;
	static int constructorCallCount;
	static int destructorCallCount;

	Foo2(Foo2* parent)
		: Base(parent)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++constructorCallCount;
	}

	~Foo2()
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		++destructorCallCount;
	}

	Foo2& operator=(const Foo2& b)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		x = b.x;
		return *this;
	}

	Foo2& operator=(Foo2&& b)
	{
		std::cout << __PRETTY_FUNCTION__ << std::endl;
		x = b.x;
		b.x = 0;
		return *this;
	}

	static void reset()
	{
		destructorCallCount = constructorCallCount = 0;
	}
};

int Foo2::constructorCallCount = 0;
int Foo2::destructorCallCount = 0;

ANKI_TEST(Object, Test)
{
	Foo2* a = new Foo2(nullptr);

	Foo2* b = new Foo2(a);
	Foo2* c = new Foo2(b);

	(void)c;

	delete a;
}

#endif

