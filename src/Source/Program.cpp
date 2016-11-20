#pragma warning(disable : 4577)
#pragma warning(disable : 4505)

//#include "Presentation.h"
#include "PresentationScene.hpp"
#pragma comment(lib, "d3d9.lib")
#pragma comment(lib, "d3dx9.lib")
#pragma comment(lib, "Comctl32.lib")

#include <thread>
#include <string>

namespace Presentation2
{
	// -----------------------
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
		explicit MenuWindow(LPCWSTR const caption = nullptr);
	};	// class MenuWindow

	// -----------------------
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
	} static *gp_AuthorsWindow = nullptr;	// class AuthorsWindow

	// -----------------------
	class PresentationWindow final : public Window
	{
	private:
		PresentationScenePtr m_Presentation;
		EngineWidgetPtr m_PresentationRenderer;

		struct PrismsControl
		{
			struct ModifierControl
			{
				WindowWidgetPtr Label;
				WindowWidgetPtr PlusButton;
				WindowWidgetPtr ValueEdit;
				WindowWidgetPtr MinusButton;
			};	// struct ModifierControl

			WindowWidgetPtr Label;
			WindowWidgetPtr AnomalousDispersionEnabled;
			ModifierControl Angle;
			ModifierControl Rotation;
#if _DEBUG
			ModifierControl PositionX, PositionY, PositionZ;
#endif
		} m_PrismsControl[2];

		WindowWidgetPtr m_OnePrismLayoutButton;
		WindowWidgetPtr m_TwoPrismsLayoutButton;
		WindowWidgetPtr m_ImageLabel;
		WindowWidgetPtr m_NormImage;
		WindowWidgetPtr m_AnomImage;
		WindowWidgetPtr m_BackButton;

	public:
		PresentationWindow();
		void Update() const
		{
			m_Presentation->OnUpdate();
		}
		void Render() const
		{
			m_Presentation->OnRender();
		}
	} static *gp_PresentationWindow = nullptr;	// class PresentationWindow



	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Windows setup.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	MenuWindow::MenuWindow(LPCWSTR const caption)
		: Window(Rect(), caption, true)
	{
		m_UniversityFacultyLogoCMC = Image({ UpperLeftPivot, 10, 10, 200, 220 }, L"../gfx/cmc1.png");
		m_UniversityFacultyLogoPH = Image({ UpperRightPivot, STANDART_DESKTOP_WIDTH - 10, 10, 200, 220 }, L"../gfx/fizfak.png");
		// -----------------------
		m_UniversityNameLabel = Label({ CenterPivot, STANDART_DESKTOP_WIDTH / 2, 40, 1500, 80 },
			L"���������� ��������������� ����������� ����� �.�. ����������\r\n"
			L"������������ ����������� �� ����� ������ ������� �������� ���������\r\n", LabelFlags::CenterAlignment, TextSize::Large);
		m_UniversityPresentationLabel = Label({ CenterPivot, STANDART_DESKTOP_WIDTH / 2, 150, 1500, 100 },
			L"���������� � ���������� ���������", LabelFlags::CenterAlignment, TextSize::VeryLarge);
		// -----------------------
		m_UniversitySeparator = HorizontalSeparator({ STANDART_DESKTOP_WIDTH / 2, 230, STANDART_DESKTOP_WIDTH, 1 });
		m_UniversityYearLabel = Label({ STANDART_DESKTOP_WIDTH / 2, STANDART_DESKTOP_HEIGHT - 30, 1500, 40 },
			L"2016 �.", LabelFlags::CenterAlignment, TextSize::NotSoLarge);
	}

	// -----------------------
	MainWindow::MainWindow()
		: MenuWindow(L"���������� ���������")
	{
		m_BeginButton = Button({ STANDART_DESKTOP_WIDTH / 2, 500, 700, 80 }, L"������", [](long)
		{
			if (gp_PresentationWindow == nullptr)
			{
				gp_PresentationWindow = new PresentationWindow();
			}
			gp_PresentationWindow->Show();
			g_MainWindow.Hide();
		}, TextSize::Large);
		m_AuthorsButton = Button({ STANDART_DESKTOP_WIDTH / 2, 590, 700, 80 }, L"������", [](long)
		{
			g_MainWindow.Hide();
			if (gp_AuthorsWindow == nullptr)
			{
				gp_AuthorsWindow = new AuthorsWindow();
			}
			gp_AuthorsWindow->Show();
		}, TextSize::Large);
		m_ExitButton = Button({ STANDART_DESKTOP_WIDTH / 2, 680, 700, 80 }, L"�����", [](long)
		{
			exit(0);
		}, TextSize::Large);
	}

	// -----------------------
	AuthorsWindow::AuthorsWindow()
		: MenuWindow(L"������")
	{
		m_GlebaPhoto = Image({ STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2, 500, 600 }, L"../gfx/gleba.png");
		m_GlebaLabel = Label({ STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 350, 500, 80 }
		, L"������� ����\r\n����������", LabelFlags::LeftAlignment, TextSize::Large);
		m_OlejaPhoto = Image({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2, 500, 600 }, L"../gfx/oleja.png");
		m_OlejaLabel = Label({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3, STANDART_DESKTOP_HEIGHT / 2 + 350, 500, 80 }
		, L"������� ����\r\n���������", LabelFlags::RightAlignment, TextSize::Large);
		// -----------------------
		m_PidorsSeparator = HorizontalSeparator({ STANDART_DESKTOP_WIDTH / 2, STANDART_DESKTOP_HEIGHT / 2 + 400, STANDART_DESKTOP_WIDTH, 1 });
		m_LecturersLabel = Label({ STANDART_DESKTOP_WIDTH / 3 + 100, STANDART_DESKTOP_HEIGHT / 2 + 450, 500 + 200, 80 }
		, L"�������:\r\n�.�. ��������, �.�. �������", LabelFlags::LeftAlignment, TextSize::Large);
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH - STANDART_DESKTOP_WIDTH / 3 + 100, STANDART_DESKTOP_HEIGHT / 2 + 450, 500 - 200, 80 }, L"�����", [](long)
		{
			g_MainWindow.Show();
			gp_AuthorsWindow->Hide();
		}, TextSize::Large);
	}

	static auto ToString(FLOAT const value)
	{
		return std::to_wstring(static_cast<LONG>(std::round(dxm::degrees(value)))) + L"�";
	}

	// -----------------------
	PresentationWindow::PresentationWindow()
		: Window(Rect(), L"�����������", true)
	{
		m_PresentationRenderer = Graphics<EngineWidget>({ STANDART_DESKTOP_WIDTH * 5 / 8, STANDART_DESKTOP_HEIGHT / 2, STANDART_DESKTOP_WIDTH * 3 / 4, STANDART_DESKTOP_HEIGHT });
		m_Presentation = m_PresentationRenderer->InitializeScene<PresentationScene>();
		// -----------------------
		m_ImageLabel = Label({ STANDART_DESKTOP_WIDTH / 8, 85, 480, 60 }, L"���������� ����������� ������ ������", LabelFlags::CenterAlignment, TextSize::Default);
		Rect const imageRect = { STANDART_DESKTOP_WIDTH / 8, 280, 480, 330 };
		m_NormImage = Image(imageRect, L"../gfx/norm-func.png");
		m_AnomImage = Image(imageRect, L"../gfx/anom-func.png");
		m_AnomImage->Hide();
		// -----------------------
		{
			auto static const PADDING = 5;
			auto static const REAL_CELL_WIDTH = STANDART_DESKTOP_WIDTH / 4 / 2;
			auto static const CELL_WIDTH = REAL_CELL_WIDTH - 2 * PADDING;
			auto static const CELL_HEIGHT = 120;
			auto static const SUBCELL_WIDTH = CELL_WIDTH;
			auto static const SUBCELL_HEIGHT = CELL_HEIGHT / 2;
			auto static const CELL_CENTER_X = STANDART_DESKTOP_WIDTH / 8;

			auto static const CellX = [](auto const i) { return REAL_CELL_WIDTH / 2 + REAL_CELL_WIDTH * i; };
			auto static const CellY = [](auto const j) { return CELL_HEIGHT / 2 + CELL_HEIGHT * j; };
			auto static const LowerSubcellY = [](auto const j) { return CellY(j) + SUBCELL_HEIGHT / 2; };
			auto static const UpperSubcellY = [](auto const j) { return CellY(j) - SUBCELL_HEIGHT / 2; };

			auto static const DEFAULT_SECOND_PRISM_ENABLED = false;

			for (auto cnt = 0; cnt < m_Presentation->m_PrismContollers.size(); ++cnt)
			{
				auto static const ROWS_PER_PRISM = 2;
				auto j = ROWS_PER_PRISM * cnt + 3;
				auto const isSecondPrism = cnt != 0;

				// Initializing the layout controls for prisms.
				auto& prism = m_Presentation->m_PrismContollers[cnt];
				auto& prismControl = m_PrismsControl[cnt];

				{	// Initializing the label of the prism control.
					auto const cellX = CELL_CENTER_X;
					auto const lowerSubcellY = LowerSubcellY(j++) + SUBCELL_HEIGHT / 2;
					prismControl.Label = Label({ cellX, lowerSubcellY, 2 * SUBCELL_WIDTH, SUBCELL_HEIGHT }
						, isSecondPrism ? L"������ ������" : L"������ ������"
						, LabelFlags::CenterAlignment, TextSize::Large);
				}

				auto static const InitializeModifierControl = [this](PrismsControl::ModifierControl& control
					, LPCWSTR const label, auto& boundedValue, auto delta, auto i, auto j1)
				{
				//	static_assert(std::is_same_v<decltype(min), std::remove_reference_t<decltype(value)>>
				//		&& std::is_same_v<decltype(min), decltype(delta)>
				//		&& std::is_same_v<decltype(min), decltype(max)>, "Different types of 'min', 'value', 'max' and 'delta' parameters.");

					// Initializing the modifier controls.
					auto const cellX = CellX(i);
					auto const lowerSubcellY = LowerSubcellY(j1);
					auto const upperSubcellY = UpperSubcellY(j1);

					auto static const BUTTON_WIDTH = SUBCELL_HEIGHT;
					auto static const BUTTON_HEIGHT = SUBCELL_HEIGHT / 2;
					auto const minusButtonX = cellX - CELL_WIDTH / 2 + BUTTON_WIDTH / 2;
					auto const minusButtonY = lowerSubcellY + SUBCELL_HEIGHT / 4;
					auto const plusButtonX = minusButtonX;
					auto const plusButtonY = lowerSubcellY - SUBCELL_HEIGHT / 4;

					auto static const TEXTEDIT_WIDTH = SUBCELL_WIDTH - BUTTON_WIDTH;
					auto static const TEXTEDIT_HEIGHT = SUBCELL_HEIGHT;
					auto const textEditX = cellX + BUTTON_WIDTH / 2;
					auto const textEditY = lowerSubcellY;

					control.Label = Label({ cellX, upperSubcellY + SUBCELL_HEIGHT / 2, SUBCELL_WIDTH, SUBCELL_HEIGHT }, label);
					control.MinusButton = Button({ minusButtonX, minusButtonY, BUTTON_WIDTH, BUTTON_HEIGHT }, L"-", [this, &boundedValue, &control, delta](long)
					{
						boundedValue.Value = dxm::clamp(boundedValue.Value - delta, boundedValue.Min, boundedValue.Max);
						control.ValueEdit->SetText(ToString(boundedValue.Value).c_str());
						m_Presentation->m_IsSceneSynced = false;
					});
					control.ValueEdit = TextEdit({ textEditX, lowerSubcellY, TEXTEDIT_WIDTH, TEXTEDIT_HEIGHT }, ToString(boundedValue.Value).c_str(), TextEditFlags::CenterAlignment);
					control.PlusButton = Button({ plusButtonX, plusButtonY, BUTTON_WIDTH, BUTTON_HEIGHT }, L"+", [this, &boundedValue, &control, delta](long)
					{
						boundedValue.Value = dxm::clamp(boundedValue.Value + delta, boundedValue.Min, boundedValue.Max);
						control.ValueEdit->SetText(ToString(boundedValue.Value).c_str());
						m_Presentation->m_IsSceneSynced = false;
					});
				};
				auto static const delta = 1.0f / 180.0f * F_PI;
				InitializeModifierControl(prismControl.Angle, L"���� ������", prism->Angle, delta, 0, j);
				InitializeModifierControl(prismControl.Rotation, L"������� ������", prism->RotationX, delta, 1, j);
				j++;
#if _DEBUG
				/*InitializeModifierControl(prismControl.PositionX, L"���������� (X)", prism.PositionMin.x, prism.Position.x, prism.PositionMax.x, 0.05f, 0, j);
				InitializeModifierControl(prismControl.PositionY, L"���������� (Y)", prism.PositionMin.y, prism.Position.y, prism.PositionMax.y, 0.05f, 1, j++);
				InitializeModifierControl(prismControl.PositionZ, L"���������� (Z)", prism.PositionMin.z, prism.Position.z, prism.PositionMax.z, 0.05f, 0, j++);*/
#endif

				if (isSecondPrism)
				{
					// Initializing the norm-anom switch for second prism.
					auto const cellX = CellX(0);
					auto const lowerSubcellY = CellY(j++);
					prismControl.AnomalousDispersionEnabled = CheckBox({ cellX, lowerSubcellY, SUBCELL_WIDTH, SUBCELL_HEIGHT }
						, L"���������� \r\n���������"
						, [this, isSecondPrism, &prism](long const enabled)
					{
						prism->Type = static_cast<PrismType>(enabled);
						m_Presentation->m_IsSceneSynced = false;
						if (isSecondPrism)
						{
							m_AnomImage->Hide(!enabled);
							m_NormImage->Show(!enabled);
						}
					}, prism->Type == PrismType::Cyanine);
				}
			}

			auto static const SwitchSecondPrismControls = [&](auto const _shown)
			{
				m_AnomImage->Hide();
				m_NormImage->Show();

				if (_shown)
				{
					m_Presentation->ResetTwoPrismsLayout();
				}
				else
				{
					m_Presentation->ResetOnePrismLayout();
				}

				for (auto i = 0; i < 2; ++i)
				{
					auto shown = i == 0 || _shown;
					auto& prism = m_Presentation->m_PrismContollers[i];
					auto& prismControl = m_PrismsControl[i];

					prism->IsEnabled = shown;

					auto const SwitchModifierControl = [&prism, &prismControl, this, shown](PrismsControl::ModifierControl& control, auto const value)
					{
						control.Label->Show(shown);
						control.PlusButton->Show(shown);
						control.MinusButton->Show(shown);
						control.ValueEdit->Show(shown);
						control.ValueEdit->SetText(ToString(value).c_str());
					};
					auto const SwitchCheckBox = [&prism, &prismControl, this, shown](WindowWidgetPtr const& checkbox, auto const checked)
					{
						if (checkbox != nullptr)
						{
							checkbox->Show(shown);
							CheckDlgButton(m_Handle, GetWindowLong(checkbox->m_Handle, GWL_ID), checked ? BST_CHECKED : BST_UNCHECKED);
						}
					};

					prismControl.Label->Show(shown);
					SwitchCheckBox(prismControl.AnomalousDispersionEnabled, prism->Type == PrismType::Cyanine);
					SwitchModifierControl(prismControl.Angle, prism->Angle);
					SwitchModifierControl(prismControl.Rotation, prism->RotationX);
#if _DEBUG
					/*SwitchModifierControl(prismControl.PositionX, prism.Position.x);
					SwitchModifierControl(prismControl.PositionY, prism.Position.y);
					SwitchModifierControl(prismControl.PositionZ, prism.Position.z);*/
#endif
				}
			};

			{	// Initializing the layout switch button.
				auto const cellY = UpperSubcellY(0);
				auto static secondPrismEnabled = DEFAULT_SECOND_PRISM_ENABLED;
				m_OnePrismLayoutButton = Button({ CellX(0), cellY, SUBCELL_WIDTH, SUBCELL_HEIGHT }, L"���� ������", [&](long const)
				{
					m_ImageLabel->SetText(L"���������� ����������� ������ ������");
					secondPrismEnabled = !secondPrismEnabled;
					SwitchSecondPrismControls(false);
				});
				m_TwoPrismsLayoutButton = Button({ CellX(1), cellY, SUBCELL_WIDTH, SUBCELL_HEIGHT }, L"��� ������", [&](long const)
				{
					m_ImageLabel->SetText(L"���������� ����������� � ���������� ��� ������ ������");
					secondPrismEnabled = !secondPrismEnabled;
					SwitchSecondPrismControls(true);
				});
			}
			SwitchSecondPrismControls(DEFAULT_SECOND_PRISM_ENABLED);
		}
		// -----------------------
		m_BackButton = Button({ STANDART_DESKTOP_WIDTH / 8, STANDART_DESKTOP_HEIGHT - 40, STANDART_DESKTOP_WIDTH / 4 - 10, 70 }, L"�����", [](long)
		{
			g_MainWindow.Show();
			gp_PresentationWindow->Hide();
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
	using namespace Presentation2;

	g_MainWindow.Show();

	atexit([]()
	{
		g_IsExitRequested = true;
	});

	// Rendering presentation in the separate thread to make buttons
	// render correctly.
	std::thread([]()
	{
		g_RenderingThreadID = std::this_thread::get_id();
		while (gp_PresentationWindow == nullptr)
		{
			Sleep(100);
		}
		while (!g_IsExitRequested)
		{
			gp_PresentationWindow->Update();
			gp_PresentationWindow->Render();
		}
		int i;
	}).detach();

	MSG msg;
	while (GetMessage(&msg, nullptr, 0, 0) && gp_PresentationWindow == nullptr)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	while (GetMessage(&msg, nullptr, 0, 0) && !g_IsExitRequested)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	delete gp_AuthorsWindow;
	delete gp_PresentationWindow;

	return 0;
}
