#ifndef RSRC_ASYNC_LOADING_REQS_HANDLER_H
#define RSRC_ASYNC_LOADING_REQS_HANDLER_H

#include <string>
#include <boost/ptr_container/ptr_list.hpp>
#include "AsyncLoader.h"
#include "Properties.h"
#include "Image.h"
#include "MeshData.h"


// Dont even think to include these files:
class Texture;
class Mesh;


/// Handles the loading requests on behalf of the resource manager. Its a different class because we want to keep the
/// source of the manager clean
class RsrcAsyncLoadingReqsHandler
{
	public:
		/// Send a loading request to an AsyncLoader
		/// @tparam Type It should be Texture or Mesh
		/// @param filename The file to load
		/// @param objToLoad Pointer to a pointer to the object to load asynchronously
		template<typename Type>
		void sendNewLoadingRequest(const char* filename, Type** objToLoad);
		
		/// Serve the finished requests. This should be called once every loop of the main loop
		/// @param maxTime The max time to spend serving finished requests. If for example there are many that need more
		/// time than the max the method will return. The pending requests will be served when it will be called again.
		/// In ms
		void postProcessFinishedRequests(uint maxTime);
		
	
	private:
		/// Request for the AsyncLoader [Base class]
		class LoadingRequestBase
		{
			public:
				enum RequestType
				{
					RT_TEXTURE,
					RT_MESH
				};

				LoadingRequestBase(const char* filename_, RequestType type_): filename(filename_), type(type_) {}

				GETTER_R(std::string, filename, getFilename)
				GETTER_R(RequestType, type, getType)

			private:
				std::string filename;
				RequestType type;
		};

		/// Request for the AsyncLoader. The combination of the template params is: <MeshData, Mesh> or <Image, Texture>
		template<typename HelperType, typename LoadType, LoadingRequestBase::RequestType type>
		struct LoadingRequest: public LoadingRequestBase
		{
			public:
				LoadingRequest(const char* filename, LoadType** objToLoad_);
				GETTER_R(HelperType, helperObj, getHelperObj)
				LoadType** getObjToLoad() {return objToLoad;}

			private:
				HelperType helperObj; ///< The object to pass to the AsyncLoader
				LoadType** objToLoad; ///< The object to serve in the main thread
		};
		
		typedef LoadingRequest<Image, Texture, LoadingRequestBase::RT_TEXTURE> TextureRequest;
		typedef LoadingRequest<MeshData, Mesh, LoadingRequestBase::RT_MESH> MeshRequest;
		
		boost::ptr_list<LoadingRequestBase> requests; ///< Loading requests

		AsyncLoader al; ///< Asynchronous loader
		
		/// @name Async loader callbacks
		/// @{
		
		/// Load image callback. Passed to al
		static void loadImageCallback(const char* filename, void* img);
		
		/// @}
};


template<typename HelperType, typename LoadType, RsrcAsyncLoadingReqsHandler::LoadingRequestBase::RequestType type_>
inline RsrcAsyncLoadingReqsHandler::LoadingRequest<HelperType, LoadType, type_>::LoadingRequest(const char* filename, 
                                                                                                LoadType** objToLoad_):
LoadingRequestBase(filename, type_),
objToLoad(objToLoad_)
{}


#endif
