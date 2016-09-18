#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

#include "yx2Framework.h"
#include "Presentation.h"
#include <string>
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

	yx2::framework::Window mainWindow({100, 100, 1280, 720});
	auto presentation = mainWindow.Direct3D9<Presentation>({0, 0, 1280, 720});

	yx2::framework::Window secndWindow({ 100, 100, 1280, 720 }, nullptr, true);
	wchar_t* buf = new wchar_t[6];
	mbstowcs_s(nullptr, buf, 6, "Начало демонстрации", _TRUNCATE);
	secndWindow.Button({300, 350,700,40 }, buf, nullptr);
	secndWindow.Button({ 300, 400, 700, 40 }, buf, nullptr);
	secndWindow.Button({ 300, 450, 700, 40 }, buf, nullptr);


	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		presentation.Update();
	}

	return 0;
}
