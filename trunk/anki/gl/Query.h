#ifndef ANKI_GL_QUERY_H
#define ANKI_GL_QUERY_H

#include <boost/scoped_ptr.hpp>


namespace anki {


class QueryImpl;


/// XXX
class Query
{
public:
	enum QueryQuestion
	{
		QQ_SAMPLES_PASSED,
		QQ_COUNT
	};

	/// @name Constructors/Destructor
	/// @{
	Query(QueryQuestion q);
	~Query();
	/// @}

	void beginQuery();
	void endQuery();

private:
	boost::scoped_ptr<QueryImpl> impl;
};


} // end namespace anki


#endif
