// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#ifndef _WIN32
#error This application supports Windows-based platforms only.
#endif // ifndef _WIN32

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif	// ifndef WIN32_LEAN_AND_MEAN 
#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN 1
#endif	// ifndef VC_EXTRALEAN 
#include <d3d9.h>

#include <vector>
#include <memory>
#include <cassert>
#include <functional>

#define ADAPI
#define ADINL
#define ADINT

namespace Presentation2
{
	namespace Utils
	{
		template<typename T>
		static T clamp(T const value, T const min, T const max)
		{
			return value < min ? min : value > max ? max : value;
		}
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Rectangles and monitors geometry.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	/** Monitor resolution API. */
	class Monitor final
	{
		enum DefaultDesktopResolution_t
		{
			DEFAULT_DESKTOP_WIDTH = 1920,
			DEFAULT_DESKTOP_HEIGHT = 1080,
		};	// enum DefaultDesktopResolution_t

		ADINT static void GetResolution(int& width, int& height)
		{
			RECT static resulution = {};
			if (resulution.right != 0)
			{
				GetWindowRect(GetDesktopWindow(), &resulution);
			}
			width = resulution.right - resulution.left;
			height = resulution.bottom - resulution.top;
		}

	public:
		ADINT static int GetWidth()
		{
			int w, h;
			GetResolution(w, h);
			return w;
		}
		ADINT static int GetHeight()
		{
			int w, h;
			GetResolution(w, h);
			return h;
		}

		ADINL static int UpscaleX(int const width)
		{
			return width * GetWidth() / DEFAULT_DESKTOP_WIDTH;
		}
		ADINL static int UpscaleY(int const height)
		{
			return height * GetHeight() / DEFAULT_DESKTOP_HEIGHT;
		}

		ADINL static int UpscaleWidth(int const width)
		{
			return max(1, UpscaleX(width));
		}
		ADINL static int UpscaleHeight(int const height)
		{
			return max(1, UpscaleY(height));
		}
	};	// class Monitor

	enum CenterPivot_t { CenterPivot };
	enum UpperLeftPivot_t { UpperLeftPivot };
	enum UpperRightPivot_t { UpperRightPivot };
	enum LowerLeftPivot_t { LowerLeftPivot };
	enum LowerRightPivot_t { LowerRightPivot };
	
	enum NoScaling_t { NoScaling };

	/** Scalable window rectangle. */
	struct Rect final
	{
		int X, Y;
		int Width, Height;

	public:
		ADINL Rect() : X(0), Y(0), Width(0), Height(0) {}
		// -----------------------
		ADINL Rect(UpperLeftPivot_t, NoScaling_t, int const x, int const y, int const w, int const h)
			: X(x), Y(y), Width(w), Height(h)
		{}
		ADINL Rect(UpperRightPivot_t, NoScaling_t, int const x, int const y, int const w, int const h)
			: X(x - w), Y(y), Width(w), Height(h)
		{}
		ADINL Rect(LowerLeftPivot_t, NoScaling_t, int const x, int const y, int const w, int const h)
			: X(x), Y(y - h), Width(w), Height(h)
		{}
		ADINL Rect(LowerRightPivot_t, NoScaling_t, int const x, int const y, int const w, int const h)
			: X(x - w), Y(y - h), Width(w), Height(h)
		{}
		ADINL Rect(CenterPivot_t, NoScaling_t, int const x, int const y, int const w, int const h)
			: X(x - w / 2), Y(y - h / 2), Width(w), Height(h)
		{}
		// -----------------------
		ADINL Rect(UpperLeftPivot_t, int const x, int const y, int const w, int const h)
			: Rect(UpperLeftPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleX(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(UpperRightPivot_t, int const x, int const y, int const w, int const h)
			: Rect(UpperRightPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleX(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(LowerLeftPivot_t, int const x, int const y, int const w, int const h)
			: Rect(LowerLeftPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleX(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(LowerRightPivot_t, int const x, int const y, int const w, int const h)
			: Rect(LowerRightPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleX(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(CenterPivot_t, int const x, int const y, int const w, int const h)
			: Rect(CenterPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleX(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		// -----------------------
		ADINL Rect(int const x, int const y, int const w, int const h)
			: Rect(CenterPivot, x, y, w, h)
		{}
	};	// struct Rect final

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Basic window widget.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	typedef std::shared_ptr<class WindowWidget> WindowWidgetPtr;
	
	enum class TextSize : DWORD
	{
		Default,
		NotSoLarge,
		Large,
		VeryLarge,
		_Count,
	};	// enum class TextSize

	/** Simple wrapper around raw WinAPI HWND handle. */
	class WindowWidget
	{
		friend class Window;
	private:
		HWND m_Hwnd;

	public:
		ADINL WindowWidget() : m_Hwnd(nullptr) {}
		ADAPI explicit WindowWidget(HWND const hwnd, TextSize const textSize = TextSize::Default)
			: m_Hwnd(hwnd)
		{
			assert(hwnd != nullptr);
			assert(textSize < TextSize::_Count);

			// Setting up font size of the widget.
			struct { int Height; HFONT Handle; } static FontsCache[static_cast<size_t>(TextSize::_Count)] = { {20}, {30}, {40}, {75} };
			auto& font = FontsCache[static_cast<size_t>(textSize)];
			if (font.Handle != nullptr)
			{
				// Lazily loading the system font.
				font.Handle = CreateFontW(Monitor::UpscaleHeight(font.Height), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
					, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas");
			}
			SendMessage(m_Hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font.Handle), TRUE);
		}
		// -----------------------
		ADAPI virtual ~WindowWidget()
		{
			DestroyWindow(m_Hwnd);
		}

	public:
		// -----------------------
		ADINL std::wstring GetText() const
		{
			std::wstring text(GetWindowTextLengthW(m_Hwnd) + 1, L'\0');
			GetWindowTextW(m_Hwnd, &text[0], text.size() + 1);
			return text;
		}
		ADINL void SetText(wchar_t const* const text) const
		{
			SetWindowTextW(m_Hwnd, text);
		}

		// -----------------------
		ADINL void Show(bool const show = true) const
		{
			ShowWindow(m_Hwnd, show ? SW_SHOW : SW_HIDE);
		}
		ADINL void Hide(bool const hide = true) const
		{
			Show(!hide);
		}
	};	// class WindowWidget

	enum class LabelFlags : DWORD
	{
		LeftAlignment = SS_LEFT,
		LeftAlignmentNoWordWrap = SS_LEFTNOWORDWRAP,
		CenterAlignment = SS_CENTER,
		RightAlignment = SS_RIGHT,
	}; // enum LabelFlags

	enum class TextEditFlags : DWORD
	{
		None = 0,
		AutoHorizontalScroll = ES_AUTOHSCROLL,
		AutoVecticalScroll = ES_AUTOVSCROLL,
		CenterAlignment = ES_CENTER,
		LeftAlignment = ES_LEFT,
		Lowercase = ES_LOWERCASE,
		Uppercase = ES_UPPERCASE,
		MultiLine = ES_MULTILINE,
		Numbers = ES_NUMBER,
		Password = ES_PASSWORD,
		ReadOnly = ES_READONLY,
		RightAlignment = ES_RIGHT,
		WantReturn = ES_WANTRETURN,
	}; // enum class TextEditFlags

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Window widgets and controls.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	typedef std::shared_ptr<class D3DWidget> D3DWidgetPtr;
	typedef std::shared_ptr<class Window> WindowPtr;
	typedef std::function<void(long)> WindowWidgetCallback;

	/** Simple wrapper around raw WinAPI Direct3D 9 device handle. */
	class D3DWidget : public WindowWidget
	{
	public:
		IDirect3DDevice9* m_Device;
		explicit D3DWidget(HWND const hwnd, IDirect3DDevice9* const device) : WindowWidget(hwnd), m_Device(device) {}

	public:
		virtual void Update() = 0;
		virtual void Render() = 0;
	};	// class D3DWidget

	/** Simple wrapper around raw WinAPI window. */
	class Window : public WindowWidget
	{
	private:
		std::vector<WindowWidgetCallback> m_Callbacks;

		// -----------------------
		ADINT WORD GenID() const
		{
			return static_cast<WORD>(m_Callbacks.size());
		}
		ADINT LRESULT static CALLBACK WindowProc(HWND const hWnd, UINT const message, WPARAM const wParam, LPARAM const lParam)
		{
			if (message == WM_CLOSE)
			{
				PostQuitMessage(0);
			}
			else if (message == WM_CLOSE)
			{
				auto const self = reinterpret_cast<Window*>(GetWindowLongW(hWnd, GWLP_USERDATA));
				if (self != nullptr)
				{
					auto const controlID = static_cast<WORD>(LOWORD(wParam));
					if (self->m_Callbacks.size() < controlID)
					{
						auto const& controlCallback = self->m_Callbacks[controlID];
						auto const isControlChecked = IsDlgButtonChecked(self->m_Hwnd, controlID);
						controlCallback(isControlChecked);
					}
				}
			}
			return DefWindowProcW(hWnd, message, wParam, lParam);
		}

	public:
		// -----------------------
		ADAPI explicit Window(Rect const& rect, wchar_t const* const caption = nullptr, bool const fullscreen = false)
		{
			WNDCLASSEX static windowClass = {};
			if (windowClass.cbSize == 0)
			{
				windowClass.cbSize = sizeof(WNDCLASSEX);
				windowClass.style = CS_HREDRAW | CS_VREDRAW;
				windowClass.lpfnWndProc = WindowProc;
				windowClass.hInstance = GetModuleHandleW(nullptr);
				windowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
				windowClass.lpszClassName = L"PresentationWindowClass1";
				RegisterClassEx(&windowClass);
			}

			auto static const hInstance = GetModuleHandleW(nullptr);
			if (fullscreen)
			{
				// Initializing fullscreen window as normal one, scaled to fullscreen.
				m_Hwnd = CreateWindowEx(0, windowClass.lpszClassName, caption
					, WS_POPUP | WS_VISIBLE | WS_OVERLAPPED
					, 0, 0, Monitor::GetWidth(), Monitor::GetHeight()
					, m_Hwnd, nullptr, hInstance, nullptr
					);
			}
			else
			{
				// Initializing normal window.
				m_Hwnd = CreateWindowEx(0, windowClass.lpszClassName, caption
					, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
					, rect.X, rect.Y, rect.Width, rect.Height
					, nullptr, nullptr, hInstance, nullptr
					);
			}
			SetWindowLongW(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			Hide();
		}

	public:
		// ***********************************************************************************************
		// Static controls.
		// ***********************************************************************************************

		// -----------------------
		ADAPI WindowWidgetPtr HorizontalSeparator(Rect const& rect) const
		{
			assert(rect.Width != 0 && rect.Height != 0);

			auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr);
			return std::make_shared<WindowWidget>(handle);
		}

		// -----------------------
		ADAPI WindowWidgetPtr Label(Rect const& rect, wchar_t const* const text, LabelFlags const flags = LabelFlags::LeftAlignment, TextSize const textSize = TextSize::Default) const
		{
			assert(rect.Width != 0 && rect.Height != 0);
			assert(text != nullptr);

			auto const handle = CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr);
			return std::make_shared<WindowWidget>(handle, textSize);
		}

		// -----------------------
		ADAPI WindowWidgetPtr Image(Rect const& rect, wchar_t const* const path) const
		{
			assert(rect.Width != 0 && rect.Height != 0);
			assert(path != nullptr);

			auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr);
			if (handle != nullptr)
			{
				auto const bitmap = LoadImageW(nullptr, path, IMAGE_BITMAP, rect.Width, rect.Height, LR_LOADFROMFILE | LR_DEFAULTSIZE);
				if (bitmap != nullptr)
				{
					SendMessageW(handle, STM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), reinterpret_cast<LPARAM>(bitmap));
				}
			}
			return std::make_shared<WindowWidget>(handle);
		}

		// ***********************************************************************************************
		// Inputs.
		// ***********************************************************************************************

		// -----------------------
		ADAPI WindowWidgetPtr TextEdit(Rect const& rect, wchar_t const* const text = nullptr, TextEditFlags const flags = TextEditFlags::None, TextSize const textSize = TextSize::Default) const
		{
			assert(rect.Width != 0 && rect.Height != 0);

			auto const handle = CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr);
			return std::make_shared<WindowWidget>(handle, textSize);
		}

		// ***********************************************************************************************
		// Buttons.
		// ***********************************************************************************************

		// -----------------------
		ADAPI WindowWidgetPtr CheckBox(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, bool const enabled = false, TextSize const textSize = TextSize::Default)
		{
			assert(rect.Width != 0 && rect.Height != 0);
			assert(text != nullptr);

			auto const id = GenID();
			m_Callbacks[id] = callback;

			auto const handle = CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE | BS_CHECKBOX
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, reinterpret_cast<HMENU>(id), nullptr, nullptr);
			CheckDlgButton(m_Hwnd, id, enabled ? BST_CHECKED : BST_UNCHECKED);
			return std::make_shared<WindowWidget>(handle, textSize);
		}

		// -----------------------
		ADAPI WindowWidgetPtr Button(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, TextSize const textSize = TextSize::Default)
		{
			assert(rect.Width != 0 && rect.Height != 0);
			assert(text != nullptr);

			auto const id = GenID();
			m_Callbacks[id] = callback;

			auto const handle = CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, reinterpret_cast<HMENU>(id), nullptr, nullptr);
			return std::make_shared<WindowWidget>(handle, textSize);
		}

		// ***********************************************************************************************
		// Direct3D 9.
		// ***********************************************************************************************

		// -----------------------
		template<typename D3DWidget_t = D3DWidget, typename D3DWidgetPtr_t = std::shared_ptr<D3DWidget_t>>
		ADAPI D3DWidgetPtr_t Direct3D9(Rect const& rect) const
		{
			static_assert(std::is_base_of_v<D3DWidget, D3DWidget_t>, "Invalid 'D3DWidget_t' template parameter type.");
			assert(rect.Width != 0 && rect.Height != 0);

			// Initializing the Direct3D 9 driver.
			auto static const direct3D = Direct3DCreate9(D3D_SDK_VERSION);

			// Creating a static widget to render into it.
			auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE
				, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr);

			// Creating the device.
			D3DPRESENT_PARAMETERS presentParameters = {};
			presentParameters.Windowed = TRUE;
			presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParameters.hDeviceWindow = handle;
			presentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
			presentParameters.BackBufferWidth = rect.Width;
			presentParameters.BackBufferHeight = rect.Height;
			presentParameters.EnableAutoDepthStencil = TRUE;
			presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
			presentParameters.MultiSampleType = D3DMULTISAMPLE_4_SAMPLES;

			LPDIRECT3DDEVICE9 device = nullptr;
			direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParameters, &device);
			return std::make_shared<D3DWidget_t>(handle, device, rect);
		}

	};	// class Window

}	// namespace Presentation2
