#ifndef VISIBILITY_TESTER_H
#define VISIBILITY_TESTER_H

#include <deque>


class VisibilityTester
{
	public:
		/// Types
		template<typedef Type>
		class Types
		{
			typedef std::deque<Type> Container;
			typedef typename Container::iterator Iterator;
			typedef typename Container::const_iterator ConstIterator;
		};

		void test(const Camera& cam);

	private:
		Scene& scene; ///< Know your father
};


#endif
