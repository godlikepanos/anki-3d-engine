#include "RsrcLoadingRequests.h"
#include "Texture.h"
#include "AsyncLoader.h"
#include "Logger.h"
#include "Globals.h"


//======================================================================================================================
// postRequest                                                                                                         =
//======================================================================================================================
void RsrcTextureLoadingRequest::postRequest(AsyncLoader& al)
{
	al.load(getFilename().c_str(), &loadImageCallback, (void*)&img);
}


//======================================================================================================================
// loadImageCallback                                                                                                   =
//======================================================================================================================
void RsrcTextureLoadingRequest::loadImageCallback(const char* filename, void* img_)
{
	Image* img = (Image*)img_;
	img->load(filename);
	//sleep(2);
}


//======================================================================================================================
// doPostLoading                                                                                                       =
//======================================================================================================================
void RsrcTextureLoadingRequest::doPostLoading()
{
	Texture* tex = new Texture;
	tex->load(img);
	*pTex = tex;
}
