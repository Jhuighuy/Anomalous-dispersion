#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

//#include "Presentation.h"
#include "PresentationWidget.h"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Comctl32.lib")

#include <thread>

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
	private:
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
	} static *g_AuthorsWindow = nullptr;	// class AuthorsWindow

	// -----------------------
	class PresentationWindow final : public Window
	{
	private:
		PresentationWidgetPtr m_Presentation;

		struct ModifierControl
		{
			WindowWidgetPtr Label;
			WindowWidgetPtr PlusButton;
			WindowWidgetPtr ValueEdit;
			WindowWidgetPtr MinusButton;
		};	// struct ModifierControl

		struct PrismsControl
		{
			WindowWidgetPtr Label;
			WindowWidgetPtr EnabledCheckbox;
			WindowWidgetPtr MaterialCombobox;
			ModifierControl Angle;
			ModifierControl PositionX, PositionY, PositionZ;
			ModifierControl RotationX, RotationZ;
		} m_PrismsControl[2];

		WindowWidgetPtr m_NormImage;
		WindowWidgetPtr m_AnomImage;
		WindowWidgetPtr m_BackButton;

	public:
		PresentationWindow();
		void Update() const
		{
			m_Presentation->Update();
		}
		void Render() const
		{
			m_Presentation->Render();
		}
	} static *g_PresentationWindow = nullptr;	// class PresentationWindow

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
		m_UniversityYearLabel = Label({ STANDART_DESKTOP_WIDTH / 2, 180, 1500, 40 },
			L"2016 г.", LabelFlags::CenterAlignment, TextSize::NotSoLarge);
	}

	// -----------------------
	MainWindow::MainWindow()
		: MenuWindow(L"Аномальная дисперсия")
	{
		m_BeginButton = Button({ STANDART_DESKTOP_WIDTH / 2, 500, 700, 80 }, L"Начало", [](long)
		{
			if (g_PresentationWindow == nullptr)
			{
				g_PresentationWindow = new PresentationWindow();
				Sleep(100);
			}
			g_PresentationWindow->Show();
			g_MainWindow.Hide();
		}, TextSize::Large);
		m_AuthorsButton = Button({ STANDART_DESKTOP_WIDTH / 2, 590, 700, 80 }, L"Авторы", [](long)
		{
			g_MainWindow.Hide();
			if (g_AuthorsWindow == nullptr)
			{
				g_AuthorsWindow = new AuthorsWindow();
			}
			g_AuthorsWindow->Show();
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
		m_LecturersLabel = Label({ STANDART_DESKTOP_WIDTH / 3 + 100, STANDART_DESKTOP_HEIGHT / 2 + 450, 500 + 200, 80 }
			, L"Лекторы:\r\nВ.П. Кандидов, А.Ю. Чикишев", LabelFlags::LeftAlignment, TextSize::Large);
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3 + 100, STANDART_DESKTOP_HEIGHT / 2 + 450, 500 - 200, 80 }, L"Назад", [](long)
		{
			g_MainWindow.Show();
			g_AuthorsWindow->Hide();
		}, TextSize::Large);
	}

	// -----------------------
	PresentationWindow::PresentationWindow()
		: Window(Rect(), L"Презентация", true)
	{
		m_Presentation = Direct3D9<PresentationWidget>({ STANDART_DESKTOP_WIDTH * 3 / 8, STANDART_DESKTOP_HEIGHT / 2, STANDART_DESKTOP_WIDTH * 3 / 4, STANDART_DESKTOP_HEIGHT});
		// -----------------------
		Rect const imageRect = { STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 8, 820, 475, 325 };
		m_NormImage = Image(imageRect, L"../gfx/norm-func.bmp");
		m_NormImage->Hide();
		m_AnomImage = Image(imageRect, L"../gfx/anom-func.bmp");
		// -----------------------
		for (auto j = 0; j < dxm::countof(m_PrismsControl); ++j)
		{
			auto& prism = m_Presentation->m_PrismRenderers[j];
			auto& prismControl = m_PrismsControl[j];

			auto static const padding = 5;
			auto static const realCellWidth = STANDART_DESKTOP_WIDTH / 4 / 2;
			auto static const cellWidth = realCellWidth - 2 * padding;
			auto static const cellHeight = 80;
			auto static const subcellWidth = cellWidth;
			auto static const subcellHeight = cellHeight / 2;

			auto static const InitializePrismLabel = [this](PrismsControl& control, auto j)
			{
				auto const cellX = STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 8;
				auto const cellY = cellHeight / 2 + cellHeight * j;

				control.Label = Label({ cellX, cellY, subcellWidth, subcellHeight }, j == 0 ? L"Первая призма" : L"Вторая призма", LabelFlags::CenterAlignment, TextSize::NotSoLarge);
			};
			auto static const InitializePrismEnabledButton = [this](Prism& prism, PrismsControl& control, auto i, auto j)
			{
				auto const cellX = realCellWidth / 2 + realCellWidth * i + STANDART_DESKTOP_WIDTH * 3 / 4;
				auto const cellY = cellHeight / 2 + cellHeight * j;
				auto const lowerSubcellY = cellY + cellHeight / 2;

				control.EnabledCheckbox = CheckBox({ cellX, lowerSubcellY, subcellWidth, subcellHeight }, L"Выключена", [&](long disabled)
				{
					prism.IsEnabled = !static_cast<bool>(disabled);
					m_Presentation->m_AreRaysSynced = false;
				});
			};
			auto static const InitializePrismCombobox = [this](Prism& prism, PrismsControl& control, auto i, auto j)
			{
				if (j == 0)
					return;

				auto const cellX = realCellWidth / 2 + realCellWidth * i + STANDART_DESKTOP_WIDTH * 3 / 4;
				auto const cellY = cellHeight / 2 + cellHeight * j;
				auto const lowerSubcellY = cellY + cellHeight / 2;
				
				control.MaterialCombobox = CheckBox({ cellX, lowerSubcellY, subcellWidth, subcellHeight }, L"Аномальная дисперсия", [&](long enabled)
				{
					prism.Type = static_cast<PrismType>(enabled);
					m_Presentation->m_AreRaysSynced = false;
					if (j != 0)
					{
						m_AnomImage->Hide(!enabled);
						m_NormImage->Show(!enabled);
					}
				}, prism.Type == PrismType::Govno);
			};
			auto static const InitializeModifierControl = [this](ModifierControl& control, auto label, auto min, auto& value, auto max, auto delta, auto i, auto j)
			{
				auto const cellX = realCellWidth / 2 + realCellWidth * i + STANDART_DESKTOP_WIDTH * 3 / 4;
				auto const cellY = cellHeight / 2 + cellHeight * j;
				auto const lowerSubcellY = cellY + cellHeight / 4;
				auto const upperSubcellY = cellY - cellHeight / 4;
				
				auto static const buttonWidth = subcellHeight;
				auto static const buttonHeight = subcellHeight / 2;
				auto const minusButtonX = cellX - cellWidth / 2 + buttonWidth / 2;
				auto const minusButtonY = lowerSubcellY + subcellHeight / 4;
				auto const plusButtonX = minusButtonX;
				auto const plusButtonY = lowerSubcellY - subcellHeight / 4;
				auto static const textEditWidth = subcellWidth - buttonWidth;
				auto static const textEditHeight = subcellHeight;
				auto const textEditX = cellX + buttonWidth / 2;
				auto const textEditY = lowerSubcellY;

				control.Label = Label({ cellX, upperSubcellY + subcellHeight / 2, subcellWidth, subcellHeight }, label);
				control.MinusButton = Button({ minusButtonX, minusButtonY, buttonWidth, buttonHeight }, L"-", [this, min, &value, max, &control, delta](long)
				{
					value = dxm::clamp(value - delta, min, max);
					control.ValueEdit->SetText(std::to_wstring(value).c_str());
					m_Presentation->m_AreRaysSynced = false;
				});
				control.ValueEdit = TextEdit({ textEditX, lowerSubcellY, textEditWidth, textEditHeight }, std::to_wstring(value).c_str(), TextEditFlags::CenterAlignment);
				control.PlusButton = Button({ plusButtonX, plusButtonY, buttonWidth, buttonHeight }, L"+", [this, min, &value, max, &control, delta](long)
				{
					value = dxm::clamp(value + delta, min, max);
					control.ValueEdit->SetText(std::to_wstring(value).c_str());
					m_Presentation->m_AreRaysSynced = false;
				});
			};

			InitializePrismLabel(prismControl, j * 4);
			InitializePrismEnabledButton(prism, prismControl, 0, j * 4);
			InitializePrismCombobox(prism, prismControl, 1, j * 4);
			InitializeModifierControl(prismControl.Angle,     L"Угол призмы",    prism.AngleMin,      prism.Angle,      prism.AngleMax,      0.05f, 1, 1 + j * 4);
			InitializeModifierControl(prismControl.RotationZ, L"Поворот призмы", prism.RotationZMin,  prism.RotationZ,  prism.RotationZMax,  0.05f, 1, 2 + j * 4);
			InitializeModifierControl(prismControl.PositionX, L"Координата (X)", prism.PositionMin.x, prism.Position.x, prism.PositionMax.x, 0.05f, 0, 1 + j * 4);
			InitializeModifierControl(prismControl.PositionY, L"Координата (Y)", prism.PositionMin.y, prism.Position.y, prism.PositionMax.y, 0.05f, 0, 2 + j * 4);
			InitializeModifierControl(prismControl.PositionZ, L"Координата (Z)", prism.PositionMin.z, prism.Position.z, prism.PositionMax.z, 0.05f, 0, 3 + j * 4);
		}
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 8, STANDART_DESKTOP_HEIGHT - 40, STANDART_DESKTOP_WIDTH / 4 - 10, 70 }, L"Назад", [](long)
		{
			g_MainWindow.Show();
			g_PresentationWindow->Hide();
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

	// Rendering presentation in the separate thread to make buttons
	// render correctly.
	std::thread([]()
	{
		while (true)
		{
			OutputDebugString(L"");
			if (Presentation1::g_PresentationWindow != nullptr)
			{
				Presentation1::g_PresentationWindow->Update();
				Presentation1::g_PresentationWindow->Render();
			}
		}
	}).detach();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete Presentation1::g_AuthorsWindow;
	delete Presentation1::g_PresentationWindow;

	return 0;
}
