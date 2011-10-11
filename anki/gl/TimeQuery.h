#ifndef ANKI_GL_TIME_QUERY_H
#define ANKI_GL_TIME_QUERY_H

#include <GL/glew.h>
#include <boost/array.hpp>


/// Used to profile the GPU. It gets the time elapsed from when the begin() is
/// called until the endAndGetTimeElapsed() method. The query causes
/// synchronization issues because it waits for the GL commands to finish. For
/// that it is slow and its not recommended for use in production code.
class TimeQuery
{
	public:
		TimeQuery();
		~TimeQuery();

		/// Begin
		void begin();

		/// End and get elapsed time. In seconds
		double end();

	private:
		/// The query state
		enum State
		{
			S_CREATED,
			S_STARTED,
			S_ENDED
		};

		boost::array<GLuint, 2> glIds; ///< GL IDs
		State state; ///< The query state. It saves us from improper use of the
		             ///< the class
};


#endif
