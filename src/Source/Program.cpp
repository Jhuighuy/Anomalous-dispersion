#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

#include "yx2Framework.h"
#include "Presentation.h"
#include <string>
#include <iostream>
#pragma comment(lib, "d3d9.lib")

namespace dxm = glm;


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

	

	yx2::framework::Window mainWindow({ 100, 100, 1280, 720 }, nullptr, true);

	std::array<yx2::framework::WindowWidget, 10> windowWidgets;
	windowWidgets[0] = mainWindow.TextEdit({ 500, 100, 700, 20 }, L"Московский Государственный Университет им. М.В. Ломоносова",
													yx2::framework::TextEditFlags::CenterAlignment);
	windowWidgets[1] = mainWindow.Image({ 10, 15, 100,100 }, L"../cmc1.bmp");
	windowWidgets[2] = mainWindow.Image({ 1525, 15, 100, 100 }, L"../fizfak.bmp");
	windowWidgets[3] = mainWindow.Button({500, 500,700,40 }, L"Начало", [&mainWindow, &windowWidgets](long)
	{
		for(auto i = 0; i < 10; ++i )
			windowWidgets[i].Destroy();

		auto presentation = mainWindow.Direct3D9<Presentation>({ 0, 0, 1280, 720 });

		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);

			presentation->Update();
		}
	
	});
	windowWidgets[4] = mainWindow.Button({ 500, 550, 700, 40 }, L"Люблю Олега", nullptr);
	windowWidgets[5] = mainWindow.Button({ 500, 600, 700, 40 }, L"Выход", [](long)
	{
		PostQuitMessage(0);
	});	
	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}
