#ifndef ANKI_RESOURCE_ASYNC_TEXTURE_RESOURCE_MANAGER_H
#define ANKI_RESOURCE_ASYNC_TEXTURE_RESOURCE_MANAGER_H

#include "anki/resource/ResourceManager.h"
#include "anki/resource/AsyncOperator.h"
#include <boost/scoped_ptr.hpp>


namespace anki {


class Texture;


/// @addtogroup resource
/// @{

/// XXX
class AsyncTextureResourceManager: public ResourceManager<Texture>
{
	public:

	private:
		/// XXX
		class Request: public AsyncOperator::Request
		{
			public:
				std::string filename;
				Image img;
				Texture** ppTex;

				Request(const char* fname, Texture**& ppTex_)
				:	filename(fname),
				 	ppTex(ppTex_)
				{}

				/// Implements AsyncOperator::Request::exec
				void exec();

				/// Implements AsyncOperator::Request::postExec
				void postExec(AsyncOperator& al);

				/// Re-implements AsyncOperator::Request::getInfo
				std::string getInfo() const;
		};

		boost::scoped_ptr<AsyncOperator> ao;

};
/// @}


} // namespace anki


#endif
