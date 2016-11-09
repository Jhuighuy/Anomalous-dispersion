#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

//#include "Presentation.h"
#include "PresentationScene.hpp"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Comctl32.lib")

#include <thread>
#include <string>
/**
* \brief Launchs the presentation.
* \param hInstance Unused WinAPI parameter.
* \param hPrevInstance Unused WinAPI parameter.
* \param lpCmdLine Unused WinAPI parameter.
* \param nCmdShow Unused WinAPI parameter.
* \return Exit code.
*/
int CALLBACK WinMain(_In_ HINSTANCE hInstance, _In_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine,
	_In_ int nCmdShow)
{
	(void)hInstance, hPrevInstance, lpCmdLine, nCmdShow;
	using namespace Presentation2;

	auto static window = std::make_shared<Window>(Rect(0,0,1,1), nullptr, true);
	window->Show();
	auto gfx = window->Graphics<EngineWidget>(Rect(UpperLeftPivot, 0, 0, 1920, 1080));
	gfx->InitializeScene<PresentationScene>();

	// Rendering presentation in the separate thread to make buttons
	// render correctly.
	std::thread([&]()
	{
		g_RenderingThreadID = std::this_thread::get_id();
		while (true)
		{
			gfx->Render();
		}
	}).detach();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		gfx->Update();
	}

	return 0;
}