#include "anki/resource/RsrcLoadingRequests.h"
#include "anki/resource/Texture.h"
#include "anki/core/AsyncLoader.h"
#include "anki/core/Logger.h"
#include "anki/core/Globals.h"


namespace anki {


//==============================================================================
// postRequest                                                                 =
//==============================================================================
void RsrcTextureLoadingRequest::postRequest(AsyncLoader& al)
{
	al.load(getFilename().c_str(), &loadImageCallback, (void*)&img);
}


//==============================================================================
// loadImageCallback                                                           =
//==============================================================================
void RsrcTextureLoadingRequest::loadImageCallback(const char* filename,
	void* img_)
{
	Image* img = (Image*)img_;
	img->load(filename);
	//sleep(2);
}


//==============================================================================
// doPostLoading                                                               =
//==============================================================================
void RsrcTextureLoadingRequest::doPostLoading()
{
	Texture* tex = new Texture;
	tex->load(img);
	*pTex = tex;
}


} // end namespace
