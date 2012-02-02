#ifndef ANKI_GL_QUERY_H
#define ANKI_GL_QUERY_H

#include <boost/scoped_ptr.hpp>
#include <boost/integer.hpp>


namespace anki {


class QueryImpl;


/// XXX
class Query
{
public:
	enum QueryQuestion
	{
		QQ_SAMPLES_PASSED,
		QQ_ANY_SAMPLES_PASSED, ///< Faster than the QQ_SAMPLES_PASSED
		QQ_TIME_ELAPSED, ///< Time elapsed in ms
		QQ_COUNT
	};

	/// @name Constructors/Destructor
	/// @{
	Query(QueryQuestion q);
	~Query();
	/// @}

	/// Start
	void beginQuery();

	/// End
	void endQuery();

	/// Get results
	/// @note Waits for operations to finish
	uint64_t getResult();

	/// Get results
	/// @note Doesn't Wait for operations to finish
	uint64_t getResultNoWait(bool& finished);

private:
	boost::scoped_ptr<QueryImpl> impl;
};


} // end namespace anki


#endif
