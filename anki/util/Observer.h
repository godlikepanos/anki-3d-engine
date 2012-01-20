#ifndef ANKI_UTIL_OBSERVER_H
#define ANKI_UTIL_OBSERVER_H

#include <boost/ptr_container/ptr_vector.hpp>


namespace anki {


/// The observer interface template
template<typename T>
struct Observer
{
	typedef T Value; ///< The type of the notification value

	virtual void notify(Value notificationVal) = 0;
};


/// Basically a container of observers
template<typename T>
class Observable
{
public:
	typedef T Value;
	typedef Observer<Value> ObserverType;
	typedef boost::ptr_vector<ObserverType> Container;

	/// Add a new observer. The Observable takes ownership of the
	/// pointer and its responsible of cleaning
	void addNewObserver(ObserverType* x)
	{
		observers.push_back(x);
	}

	/// Notify all observers
	void notifyAll(Value x)
	{
		for(typename Container::iterator it = observers.begin();
			it != observers.end(); ++it)
		{
			(*it).notify(x);
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


/// An over-qualified observer
template<typename ObservingType, typename Value,
	void (ObservingType::*method)(Value)>
struct Observing: Observer<Value>
{
	ObservingType* reveiver;

	Observing(ObservingType* reveiver_)
		: reveiver(reveiver_)
	{}

	void notify(Value x)
	{
		(reveiver->*method)(x);
	}
};


/// If a class has slots it should include this
#define ANKI_OBSERVING(_class) \
	typedef _class ObservingType;


/// Define a signal
#define ANKI_SIGNAL(_type, _name) \
	Observable<_type> _name;


/// It doesn't do anything. Its purpose is to make the code more understandable
#define ANKI_EMIT this->


/// Define a slot. This should follow the method declaration
#define ANKI_SLOT(_name, _type) \
	typedef Observing<ObservingType, _type, &ObservingType::_name> \
		Observing_##_name;


/// Connect a signal to a slot
#define ANKI_CONNECT(_sender, _signal, _reveiver, _slot) \
	 (_sender)->_signal.addNewObserver(new Observing_##_slot(_reveiver))


} // namespace anki


#endif
