// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Maathematics & Cybernetics, MSU.

#pragma once
#ifndef _WIN32
#error YX2 engine supports Windows-based platforms only.
#endif // ifndef _WIN32

#include <map>
#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <Windows.h>
#include <assert.h>
#include <d3d9.h>
#include <wtypes.h>
#include <winuser.h>

#define STANDART_DESKTOP_WIDTH 1920
#define STANDART_DESKTOP_HEIGHT 1080

namespace Presentation1
{

	inline void GetDesktopResolution(unsigned& horizontal, unsigned& vertical)
	{
		RECT desktop;
		// Get a handle to the desktop window
		const HWND hDesktop = GetDesktopWindow();
		// Get the size of screen to the variable desktop
		GetWindowRect(hDesktop, &desktop);
		// The top left corner will have coordinates (0,0)
		// and the bottom right corner will have coordinates
		// (horizontal, vertical)
		horizontal = desktop.right;
		vertical = desktop.bottom;
	}

	struct Rect
	{
	public:
		int  x;
		int  y;
		unsigned w;
		unsigned h;

		Rect() : x(0), y(0), w(0), h(0) {}
		//Rect(int const x, int const y, unsigned const w, unsigned const h) : x(x), y(y), w(w), h(h) {}

		Rect(int const x1, int const y1, unsigned const w1, unsigned const h1)
		{
			unsigned horizontal, vertical;
			GetDesktopResolution(horizontal, vertical);
			x = round(x1 * horizontal / STANDART_DESKTOP_WIDTH);
			y = round(y1 * vertical / STANDART_DESKTOP_HEIGHT);
			w = round(w1 * horizontal / STANDART_DESKTOP_WIDTH);
			h = round(h1 * vertical / STANDART_DESKTOP_HEIGHT);

		}

		static Rect Fullscreen()
		{
			unsigned horizontal, vertical;
			GetDesktopResolution(horizontal, vertical);
			return Rect(0, 0, horizontal, vertical);
		}
	}; // struct Rect

	enum class TextSize : DWORD
	{
		Default,
		Large,
		VeryLarge,
		_Count,
	};	// enum class TextSize

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

	typedef WORD WindowWidgetHash;
	typedef std::function<void(long)> WindowWidgetCallback;

	/**
	* \brief Tiny REFERENCE wrapper around the raw WinAPI UI widget.
	*/
	typedef std::shared_ptr<class WindowWidget> WindowWidgetPtr;
	class WindowWidget
	{
		friend class Window;

	private:
		HWND m_hwnd;

		HFONT static GetFont(TextSize const textSize)
		{
			assert(textSize < TextSize::_Count);
			switch (textSize)
			{
				case TextSize::Default:
				{
					HFONT static defaultFont = nullptr;
					if (defaultFont == nullptr)
					{
						defaultFont = CreateFont(20, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
							, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS
							, L"Times New Roman");
					}
					return defaultFont;
				}
				case TextSize::Large:
				{
					HFONT static largeFont = nullptr;
					if (largeFont == nullptr)
					{
						largeFont = CreateFont(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
							, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS
							, L"Times New Roman");
					}
					return largeFont;
				}
				case TextSize::VeryLarge:
				{
					HFONT static veryLargeFont = nullptr;
					if (veryLargeFont == nullptr)
					{
						veryLargeFont = CreateFont(75, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
							, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS
							, L"Times New Roman");
					}
					return veryLargeFont;
				}
				default:
					assert(false);
					return nullptr;
			}
		}

	public:
		WindowWidget() : m_hwnd(nullptr) {}
		explicit WindowWidget(HWND const hwnd, TextSize const textSize = TextSize::Default) 
			: m_hwnd(hwnd)
		{
			SendMessage(m_hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(GetFont(textSize)), TRUE);
		}
		~WindowWidget()
		{
			DestroyWindow(m_hwnd);
		}

	public:
		std::wstring GetText() const
		{
			std::wstring text(GetWindowTextLengthW(m_hwnd) + 1, L'\0');
			GetWindowTextW(m_hwnd, &text[0], text.size() + 1);
			return text;
		}
		void SetText(wchar_t const* const text) const
		{
			SetWindowTextW(m_hwnd, text);
		}

		void Show(bool const show = true) const
		{
			ShowWindow(m_hwnd, show ? SW_SHOW : SW_HIDE);
		}
		void Hide(bool const hide = true) const
		{
			Show(!hide);
		}
	}; // class WindowWidget

	typedef std::shared_ptr<class D3DWidget> D3DWidgetPtr;
	class D3DWidget : public WindowWidget
	{
	public:
		IDirect3DDevice9* m_Device;
		explicit D3DWidget(HWND const hwnd, IDirect3DDevice9* const device) : WindowWidget(hwnd), m_Device(device) {}
	};	// class D3DWidget

	/**
	 * \brief Tiny wrapper around raw WinAPI functions for the UI.
	 * Simply wraps all WinAPI calls for UI building. Supports DirectX 9 interactions.
	 */
	typedef std::shared_ptr<Window> WindowPtr;
	class Window : public WindowWidget
	{
	private:
		std::map<WindowWidgetHash, WindowWidgetCallback> m_Callbacks;

		// ----------------------------------------------------------------
		// ----------------------------------------------------------------

		LRESULT static CALLBACK WindowProc(HWND const hWnd, UINT const message, WPARAM const wParam, LPARAM const lParam)
		{
			auto const self = reinterpret_cast<Window*>(GetWindowLongW(hWnd, GWLP_USERDATA));
			if (message == WM_CLOSE)
				PostQuitMessage(0);
			return self != nullptr ? self->SelfWindowProc(message, wParam, lParam) : DefWindowProcW(hWnd, message, wParam, lParam);
		}
		LRESULT SelfWindowProc(UINT const message, WPARAM const wParam, LPARAM const lParam)
		{
			switch (message)
			{
				case WM_COMMAND:
				{
					auto const hash = static_cast<WindowWidgetHash>(LOWORD(wParam));
					if (m_Callbacks.count(hash) != 0 && m_Callbacks[hash] != nullptr)
					{
						m_Callbacks[hash](0);
					}
				} break;
				default:
					break;
			}
			return DefWindowProcW(m_hwnd, message, wParam, lParam);
		}

		auto static Hash(wchar_t const* const string)
		{
			return static_cast<WindowWidgetHash>(std::hash<wchar_t const*>()(string));
		}

	public:
		explicit Window(Rect const& rect, wchar_t const* const caption = nullptr, bool const fullscreen = false)
		{
			WNDCLASSEX static windowClass;
			if (windowClass.cbSize == 0)
			{
				windowClass.cbSize = sizeof(WNDCLASSEX);
				windowClass.style = CS_HREDRAW | CS_VREDRAW;
				windowClass.lpfnWndProc = WindowProc;
				windowClass.hInstance = GetModuleHandleW(nullptr);
				windowClass.hbrBackground = /*reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1)*/(HBRUSH)COLOR_WINDOW;
				windowClass.lpszClassName = L"PresentationWindowClass1";
				RegisterClassEx(&windowClass);
			}

			auto const hInstance = GetModuleHandleW(nullptr);
			if (fullscreen)
			{
				auto const& fullscreenRect = Rect::Fullscreen();
				m_hwnd = CreateWindowEx(0, windowClass.lpszClassName, caption,
					WS_POPUP | WS_VISIBLE | WS_OVERLAPPED,
					fullscreenRect.x, fullscreenRect.y, fullscreenRect.w, fullscreenRect.h,
					m_hwnd, nullptr, hInstance, nullptr);
			}
			else
			{
				m_hwnd = CreateWindowEx(0, windowClass.lpszClassName, caption, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
					rect.x, rect.y, rect.w, rect.h,
					nullptr, nullptr, hInstance, nullptr);
			}
			SetWindowLongW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			Hide();
		}

	public:
		// ***********************************************************************************************
		// Static controls.
		// ***********************************************************************************************

		WindowWidgetPtr HorizontalSeparator(Rect const& rect) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return std::make_shared<WindowWidget>(
				CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ
					, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		WindowWidgetPtr VerticalSeparator(Rect const& rect) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return std::make_shared<WindowWidget>(
				CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT
					, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		WindowWidgetPtr Label(Rect const& rect, wchar_t const* const text, LabelFlags const flags = LabelFlags::LeftAlignment, TextSize const textSize = TextSize::Default) const
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(text != nullptr);

			return std::make_shared<WindowWidget>(
				CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
					, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr), textSize);
		}

		WindowWidgetPtr Image(Rect const& rect, wchar_t const* const path) const
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(path != nullptr);

			auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP
				, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr);
			if (handle != nullptr)
			{
				auto const bitmap = LoadImageW(nullptr, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE);
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

		WindowWidgetPtr TextEdit(Rect const& rect, wchar_t const* const text = nullptr, TextEditFlags const flags = TextEditFlags::None, TextSize const textSize = TextSize::Default) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return std::make_shared<WindowWidget>(
				CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
					, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr), textSize);
		}

		// ***********************************************************************************************
		// Buttons.
		// ***********************************************************************************************

		WindowWidgetPtr Button(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, TextSize const textSize = TextSize::Default)
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(text != nullptr);

			auto const hash = Hash(text);
			m_Callbacks[hash] = callback;
			return std::make_shared<WindowWidget>(
				CreateWindowW(L"Button", text, WS_VISIBLE | WS_CHILD
					, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, reinterpret_cast<HMENU>(hash), nullptr, nullptr), textSize);
		}

		// ***********************************************************************************************
		// Direct3D 9.
		// ***********************************************************************************************

		template<typename TD3DWidget = D3DWidget>
		std::shared_ptr<TD3DWidget> Direct3D9(Rect const& rect) const
		{
			auto const direct3D = Direct3DCreate9(D3D_SDK_VERSION);
			auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE
				, rect.x - rect.w / 2, rect.y - rect.h / 2, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr);

			LPDIRECT3DDEVICE9 device;
			D3DPRESENT_PARAMETERS presentParameters = {};
			presentParameters.Windowed = TRUE;
			presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParameters.hDeviceWindow = handle;
			presentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
			presentParameters.BackBufferWidth = rect.w;
			presentParameters.BackBufferHeight = rect.h;
			presentParameters.EnableAutoDepthStencil = TRUE;
			presentParameters.AutoDepthStencilFormat = D3DFMT_D16;
			presentParameters.MultiSampleType = D3DMULTISAMPLE_8_SAMPLES;

			direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParameters, &device);
			return std::make_shared<TD3DWidget>(handle, device, rect);
		}

	}; // class Window

}	// namespace Presentation1
