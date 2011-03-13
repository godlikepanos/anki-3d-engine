#include "RsrcAsyncLoadingReqsHandler.h"
#include "Texture.h"
#include "Logger.h"
#include "HighRezTimer.h"
#include "Assert.h"


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
// postProcessFinishedRequests                                                                                         =
//======================================================================================================================
void RsrcAsyncLoadingReqsHandler::postProcessFinishedRequests(uint maxTime)
{
	HighRezTimer t;
	t.start();
	frameServedRequestsNum = 0;

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
		ASSERT(filename == req.getFilename());

		switch(req.getType())
		{
			case LoadingRequestBase::RT_TEXTURE:
			{
				TextureRequest& texReq = static_cast<TextureRequest&>(req);
				Texture* tex = new Texture;
				tex->load(texReq.getHelperObj());
				*texReq.getObjToLoad() = tex;
				break;
			}
				
			case LoadingRequestBase::RT_MESH:
				/// @todo handle it
				break;
		}
		
		++frameServedRequestsNum;

		requests.pop_front();

		// Leave if you passed the max time
		if(t.getElapsedTime() >= maxTime)
		{
			break;
		}
	}

	if(frameServedRequestsNum > 0)
	{
		INFO(frameServedRequestsNum << " requests served. Time: " << t.getElapsedTime() << ", max time: " << maxTime);
	}
}


//======================================================================================================================
// loadImageCallback                                                                                                   =
//======================================================================================================================
void RsrcAsyncLoadingReqsHandler::loadImageCallback(const char* filename, void* img_)
{
	Image* img = (Image*)img_;
	img->load(filename);
	//sleep(2);
}

