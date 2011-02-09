#include "RsrcAsyncLoadingReqsHandler.h"
#include "Texture.h"


//======================================================================================================================
// sendNewLoadingRequest <Texture>                                                                                     =
//======================================================================================================================
template<>
void RsrcAsyncLoadingReqsHandler::sendNewLoadingRequest<Texture>(const char* filename, Texture** objToLoad)
{
	TextureRequest* req = new TextureRequest(filename, objToLoad);
	requests.push_back(req);
	al.load(filename, &loadImageCallback, (void*)&req->getHelperObj());
}


//======================================================================================================================
// serveFinishedRequests                                                                                               =
//======================================================================================================================
void RsrcAsyncLoadingReqsHandler::serveFinishedRequests(uint /*maxTime*/)
{
	while(1)
	{
		std::string filename;
		void* storage = NULL;
		bool ok;
		
		if(!al.pollForFinished(filename, storage, ok)) // If no pending 
		{
			break;
		}

		LoadingRequestBase& req = requests.front();	
		RASSERT_THROW_EXCEPTION(filename != req.getFilename());

		switch(req.getType())
		{
			case LoadingRequestBase::RT_TEXTURE:
			{
				TextureRequest& texReq = static_cast<TextureRequest&>(req);
				Texture* tex = new Texture;
				tex->load(texReq.getHelperObj());
				//*texReq.getObjToLoad()->load(texReq.getHelperObj());
				break;
			}
				
			case LoadingRequestBase::RT_MESH:
				/// @todo handle it
				break;
		}
		
		requests.pop_front();
	}
}


//======================================================================================================================
// loadImageCallback                                                                                                   =
//======================================================================================================================
void RsrcAsyncLoadingReqsHandler::loadImageCallback(const char* filename, void* img_)
{
	Image* img = (Image*)img_;
	img->load(filename);
}

