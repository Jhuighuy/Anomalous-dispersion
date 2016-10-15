#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

//#include "Presentation.h"
#include "PresentationWidget.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")

namespace Presentation1
{
	class MenuWindow : public Window
	{
	private:
		WindowWidgetPtr m_UniversityFacultyLogoCMC;
		WindowWidgetPtr m_UniversityFacultyLogoPH;
		WindowWidgetPtr m_UniversityNameLabel;
		WindowWidgetPtr m_UniversityPresentationLabel;
		WindowWidgetPtr m_UniversitySeparator;
		WindowWidgetPtr m_UniversityYearLabel;

	public:
		explicit MenuWindow(wchar_t const* const caption = nullptr);
	};	// class MenuWindow

	class MainWindow final : public MenuWindow
	{
		WindowWidgetPtr m_BeginButton;
		WindowWidgetPtr m_AuthorsButton;
		WindowWidgetPtr m_ExitButton;

	public:
		MainWindow();
	} static g_MainWindow;	// class MainWindow

	// -----------------------
	class AuthorsWindow final : public MenuWindow
	{
	private:
		WindowWidgetPtr m_GlebaPhoto;
		WindowWidgetPtr m_GlebaLabel;
		WindowWidgetPtr m_OlejaPhoto;
		WindowWidgetPtr m_OlejaLabel;
		WindowWidgetPtr m_PidorsSeparator;

		WindowWidgetPtr m_LecturersLabel;

		WindowWidgetPtr m_BackButton;

	public:
		AuthorsWindow();
	} static g_AuthorsWindow;	// class AuthorsWindow

	// -----------------------
	class PresentationWindow final : public Window
	{
	private:
		PresentationWidgetPtr m_Presentation;
		WindowWidgetPtr m_BackButton;

		struct ModifierControl
		{
			WindowWidgetPtr PlusButton;
			WindowWidgetPtr ValueEdit;
			WindowWidgetPtr MinusButton;
		};	// struct ModifierControl

		struct
		{
			WindowWidgetPtr EnabledButton;
			ModifierControl Angle;
			ModifierControl PositionX, PositionY, PositionZ;
			ModifierControl RotationX, RotationZ;
		} m_PrismsControl[2];

	public:
		PresentationWindow();
		void Update()
		{
			m_Presentation->Update();
		}
	} static g_PresentationWindow;	// class PresentationWindow

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Windows setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	MenuWindow::MenuWindow(wchar_t const* const caption)
		: Window(Rect(), caption, true)
	{
		m_UniversityFacultyLogoCMC = Image({ 105, 105, 150, 150 }, L"../gfx/cmc1.bmp");
		m_UniversityFacultyLogoPH = Image({ STANDART_DESKTOP_WIDTH - 105, 105, 150, 150 }, L"../gfx/fizfak.bmp");
		// -----------------------
		m_UniversityNameLabel = Label({ STANDART_DESKTOP_WIDTH / 2, 40, 1500, 80 },
			L"Московский Государственный Университет им. М.В. Ломоносова\r\n"
			L"Компьютерная презентация по курсу «Физика Волновых Процессов»\r\n", LabelFlags::CenterAlignment, TextSize::Large);
		m_UniversityPresentationLabel = Label({ STANDART_DESKTOP_WIDTH / 2, 130, 1500, 100 },
			L"Нормальная и аномальная дисперсия", LabelFlags::CenterAlignment, TextSize::VeryLarge);
		// -----------------------
		m_UniversitySeparator = HorizontalSeparator({ STANDART_DESKTOP_WIDTH / 2, 210, STANDART_DESKTOP_WIDTH, 1 });
		m_UniversityYearLabel = Label({ STANDART_DESKTOP_WIDTH / 2, 190, 1500, 20 },
			L"2016 г.", LabelFlags::CenterAlignment);
	}

	// -----------------------
	MainWindow::MainWindow()
		: MenuWindow(L"Аномальная дисперсия")
	{
		m_BeginButton = Button({ STANDART_DESKTOP_WIDTH / 2, 500, 700, 80 }, L"Начало", [](long)
		{
			g_MainWindow.Hide();
			g_PresentationWindow.Show();
		}, TextSize::Large);
		m_AuthorsButton = Button({ STANDART_DESKTOP_WIDTH / 2, 590, 700, 80 }, L"Авторы", [](long)
		{
			g_MainWindow.Hide();
			g_AuthorsWindow.Show();
		}, TextSize::Large);
		m_ExitButton = Button({ STANDART_DESKTOP_WIDTH / 2, 680, 700, 80 }, L"Выход", [](long)
		{
			exit(0);
		}, TextSize::Large);
	}

	// -----------------------
	AuthorsWindow::AuthorsWindow()
		: MenuWindow(L"Авторы")
	{
		m_GlebaPhoto = Image({ STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2, 500, 600 }, L"../gfx/gleba.bmp");
		m_GlebaLabel = Label({ STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 350, 500, 80 }
			, L"Плаксин Глеб\r\nМаксимович", LabelFlags::LeftAlignment, TextSize::Large);
		m_OlejaPhoto = Image({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2, 500, 600 }, L"../gfx/oleja.bmp");
		m_OlejaLabel = Label({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 350, 500, 80 }
			, L"Бутаков Олег\r\nБорисович", LabelFlags::RightAlignment, TextSize::Large);
		// -----------------------
		m_PidorsSeparator = HorizontalSeparator({ STANDART_DESKTOP_WIDTH / 2, STANDART_DESKTOP_HEIGHT / 2 + 400, STANDART_DESKTOP_WIDTH, 1 });
		m_LecturersLabel = Label({ STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 450, 500, 80 }
			, L"Лекторы:\r\nВ.П. Кандидов, А.Ю. Чикишев", LabelFlags::LeftAlignment, TextSize::Large);
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 450, 500, 80 }, L"Назад", [](long)
		{
			g_AuthorsWindow.Hide();
			g_MainWindow.Show();
		}, TextSize::Large);
	}

	// -----------------------
	PresentationWindow::PresentationWindow()
		: Window(Rect(), L"Презентация", true)
	{
		m_Presentation = Direct3D9<PresentationWidget>({ STANDART_DESKTOP_WIDTH * 3 / 8, STANDART_DESKTOP_HEIGHT / 2, STANDART_DESKTOP_WIDTH * 3 / 4, STANDART_DESKTOP_HEIGHT});
		// -----------------------
		for (auto i = 0; i < /*dxm::countof(m_PrismsControl)*/1; ++i)
		{
			auto& prism = m_Presentation->m_PrismRenderers[i];	// @todo
			auto& prismControl = m_PrismsControl[i];

			auto static const InitializeModifierControl = [i, this](ModifierControl& control, auto& value, auto eps, auto j)
			{
				auto static const buttonSize = 40u;
				auto static const textEditWidth = 250u;
				auto static const spacingSize = 10;

				auto const topOffset = j * (spacingSize + (int)buttonSize);
				auto static const leftButtonOffset = STANDART_DESKTOP_WIDTH * 3 / 4 + (int)(buttonSize + spacingSize);
				auto static const rightButtonOffset = STANDART_DESKTOP_WIDTH - (int)(buttonSize + spacingSize);
				auto static const textEditOffset = (leftButtonOffset + rightButtonOffset) / 2;
				control.MinusButton = Button({leftButtonOffset, topOffset, buttonSize, buttonSize}, L"-", [&value, &control, eps](long)
				{
					value -= eps;
					control.ValueEdit->SetText(std::to_wstring(value).c_str());
				});
				control.ValueEdit = TextEdit({ textEditOffset, topOffset, textEditWidth, buttonSize }, std::to_wstring(value).c_str(), TextEditFlags::CenterAlignment);
				control.PlusButton = Button({ rightButtonOffset, topOffset, buttonSize, buttonSize }, L"+", [&value, &control, eps](long)
				{
					value += eps;
					control.ValueEdit->SetText(std::to_wstring(value).c_str());
				});
			};
			InitializeModifierControl(prismControl.Angle, prism.Angle, 0.25f, 1);
			InitializeModifierControl(prismControl.PositionX, prism.Position.x, 0.25f, 2);
			InitializeModifierControl(prismControl.PositionY, prism.Position.y, 0.25f, 3);
			InitializeModifierControl(prismControl.PositionZ, prism.Position.z, 0.25f, 4);
		}
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 8, STANDART_DESKTOP_HEIGHT - 40, STANDART_DESKTOP_WIDTH / 4, 80 }, L"Назад", [](long)
		{
			g_PresentationWindow.Hide();
			g_MainWindow.Show();
		}, TextSize::Large);
	}

}	// namespace Presentation1

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

	Presentation1::g_MainWindow.Show();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);

		Presentation1::g_PresentationWindow.Update();
	}

	return 0;
}
