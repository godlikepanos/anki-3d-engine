#include "RsrcAsyncLoadingReqsHandler.h"
#include "Resources/Texture.h"
#include "Core/Logger.h"
#include "Util/HighRezTimer.h"
#include "Util/Assert.h"
#include "Core/Globals.h"


//==============================================================================
// sendNewLoadingRequest <Texture>                                             =
//==============================================================================
template<>
void RsrcAsyncLoadingReqsHandler::sendNewLoadingRequest<Texture>(
	const char* filename, Texture** objToLoad)
{
	RsrcTextureLoadingRequest* req = new RsrcTextureLoadingRequest(filename,
		objToLoad);
	requests.push_back(req);
	req->postRequest(al);
}


//==============================================================================
// postProcessFinishedRequests                                                 =
//==============================================================================
void RsrcAsyncLoadingReqsHandler::postProcessFinishedRequests(float maxTime)
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

		RsrcLoadingRequestBase& req = requests.front();
		ASSERT(filename == req.getFilename());

		try
		{
			req.doPostLoading();
		}
		catch(std::exception& e)
		{
			ERROR("Post-loading failed for \"" << filename << "\": " <<
				e.what());
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
		INFO(frameServedRequestsNum << " requests served. Time: " <<
			t.getElapsedTime() << ", max time: " << maxTime);
	}
}

