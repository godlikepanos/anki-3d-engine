#ifndef ANKI_UTIL_OBSERVER_H
#define ANKI_UTIL_OBSERVER_H

#include "anki/util/Vector.h"
#include "anki/util/Functions.h"

namespace anki {

/// @addtogroup util_patterns
/// @{

/// The observer interface template
template<typename T>
struct Observer
{
	typedef T Value; ///< The type of the notification value

	virtual ~Observer()
	{}

	virtual void notify(Value notificationVal) = 0;
};

/// An over-qualified observer
template<typename ObservingClass, typename Value,
	void (ObservingClass::*method)(Value)>
struct SuperObserver: Observer<Value>
{
	ObservingClass* receiver;

	SuperObserver(ObservingClass* receiver_)
		: receiver(receiver_)
	{
		ANKI_ASSERT(receiver != nullptr);
	}

	void notify(Value x)
	{
		(receiver->*method)(x);
	}
};

/// Basically a container of observers
template<typename T, typename Alloc = Allocator<Observer<T>*>>
class Observable
{
public:
	typedef T Value;
	typedef Observer<Value> ObserverType;
	typedef Vector<ObserverType*, Alloc> Container;

	Observable(const Alloc& alloc = Alloc())
		: observers(alloc)
	{}

	~Observable()
	{
		Alloc alloc = observers.get_allocator();
		for(ObserverType* x : observers)
		{
			alloc.destroy(x);
			alloc.deallocate(x, 1);
		}
	}

	/// XXX
	template<typename Observer, typename... Args>
	Observer* newObserver(Args&&... args)
	{
		typedef typename Alloc::template rebind<Observer>::other UberAlloc;

		UberAlloc alloc(observers.get_allocator());

		Observer* ptr = alloc.allocate(1);
		alloc.construct(ptr, std::forward<Args>(args)...);

		return ptr;
	}

	/// Add a new observer. The Observable takes ownership of the
	/// pointer and its responsible of cleaning
	void addNewObserver(ObserverType* x)
	{
		observers.push_back(x);
	}

	/// Notify all observers
	void notifyAll(Value x)
	{
		for(ObserverType* obs : observers)
		{
			obs->notify(x);
		}
	}

	/// Alias to notifyAll
	void operator()(Value x)
	{
		notifyAll(x);
	}

private:
	Container observers;
};

/// If a class has slots it should include this
/// @code
/// class Foo {
/// 	ANKI_HAS_SLOTS(Foo)
/// };
/// @endcode
#define ANKI_HAS_SLOTS(_class) \
	typedef _class ObservingType;

/// Define a slot. This should follow the method declaration
/// @code
/// class Foo {
/// 	ANKI_HAS_SLOTS(Foo)
///
/// 	void slot(const float&)
/// 	{...}
/// 	ANKI_SLOT(updateZFar, const float&)
/// };
/// @endcode
#define ANKI_SLOT(_slot, _type) \
	typedef SuperObserver<ObservingType, _type, &ObservingType::_slot> \
		Observing_##_slot;

/// Define a signal
#define ANKI_SIGNAL(_type, _name) \
	Observable<_type> _name;

/// Define a signal with allocator
#define ANKI_SIGNAL_ALLOCATOR(_Type, _name, _AllocType) \
	Observable<_Type, _AllocType> _name;

/// It doesn't do anything. Its purpose is to make the code more understandable
#define ANKI_EMIT this->

/// Connect a signal to a slot
/// @note Use RemovePointer so you can be able to use the macro outside of the
///       _receiver body
#define ANKI_CONNECT(_sender, _signal, _receiver, _slot) \
	do { \
		auto observer = (_sender)->_signal.newObserver< \
			RemovePointer<decltype(_receiver)>::Type::Observing_##_slot>( \
			_receiver); \
		(_sender)->_signal.addNewObserver(observer); \
	} while(false);

/// @}

} // end namespace anki

#endif
