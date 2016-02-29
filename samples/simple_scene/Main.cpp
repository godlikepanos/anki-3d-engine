#include <cstdio>
#include <anki/AnKi.h>

using namespace anki;

class MyApp : public App
{
public:
	PerspectiveCamera* m_cam = nullptr;

	Error init();
	Error userMainLoop(Bool& quit) override;
};

MyApp* app;

//==============================================================================
Error MyApp::init()
{
	// TODO

	return ErrorCode::NONE;
}

//==============================================================================
Error MyApp::userMainLoop(Bool& quit)
{
	// TODO

	return ErrorCode::NONE;
}

//==============================================================================
int main(int argc, char* argv[])
{
	// TODO

	return 0;
}
