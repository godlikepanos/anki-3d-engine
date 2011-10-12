#include "anki/resource/RsrcAsyncLoadingReqsHandler.h"
#include "anki/resource/Texture.h"
#include "anki/core/Logger.h"
#include "anki/util/HighRezTimer.h"
#include "anki/util/Assert.h"
#include "anki/core/Globals.h"


namespace anki {


/*
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
			ANKI_ERROR("Post-loading failed for \"" << filename << "\": " <<
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
		ANKI_INFO(frameServedRequestsNum << " requests served. Time: " <<
			t.getElapsedTime() << ", max time: " << maxTime);
	}
}

*/


} // end namespace
