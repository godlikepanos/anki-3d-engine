#ifndef RSRC_LOADING_REQUESTS_H
#define RSRC_LOADING_REQUESTS_H

#include <string>
#include "Accessors.h"
#include "Image.h"


class AsyncLoader;
// Dont even think to include these files:
class Texture;


/// Request for the AsyncLoader [Base class]
class RsrcLoadingRequestBase
{
	public:
		RsrcLoadingRequestBase(const char* filename_): filename(filename_) {}
		GETTER_R(std::string, filename, getFilename)
		virtual void postRequest(AsyncLoader& al) = 0;
		virtual void doPostLoading() = 0;

	private:
		std::string filename;
};


/// @todo
class RsrcTextureLoadingRequest: public RsrcLoadingRequestBase
{
	public:
		RsrcTextureLoadingRequest(const char* filename_, Texture** pTex_);

		void postRequest(AsyncLoader& al);
		void doPostLoading();

	private:
		Image img;
		Texture** pTex;

		/// Load image callback. Passed to AsyncLoader
		static void loadImageCallback(const char* filename, void* img);
};


inline RsrcTextureLoadingRequest::RsrcTextureLoadingRequest(const char* filename_, Texture** pTex_):
	RsrcLoadingRequestBase(filename_),
	pTex(pTex_)
{}


#endif
