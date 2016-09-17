#pragma once

#include <Windows.h>
#include <d3d9.h>

#include <algorithm>
#include <cassert>
#include <functional>
#include <memory>
#include <utility>
#include <vector>
#define UI_API

namespace UI
{
	struct Rect
	{
	public:
		int const x;
		int const y;
		unsigned const w;
		unsigned const h;

		Rect() : x(0), y(0), w(0), h(0) {}
		Rect(int const x, int const y, unsigned const w, unsigned const h) : x(x), y(y), w(w), h(h)
		{
		}
	}; // struct Rect

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
	class WindowWidget
	{
		friend class Window;

	private:
		HWND m_hwnd;

	public:
		WindowWidget() : m_hwnd(nullptr) {}
		explicit WindowWidget(HWND const hwnd) : m_hwnd(hwnd) {}

	public:
		UI_API std::wstring GetText() const
		{
			std::wstring text(GetWindowTextLengthW(m_hwnd) + 1, L'\0');
			GetWindowTextW(m_hwnd, &text[0], text.size() + 1);
			return text;
		}
		UI_API void SetText(wchar_t const* const text) const { SetWindowTextW(m_hwnd, text); }

	}; // class WindowWidget

	/**
	 * \brief Tiny wrapper around raw WinAPI functions for the UI.
	 * Simply wraps all WinAPI calls for UI building. Supports DirectX 9 interactions.
	 */
	class Window : public WindowWidget
	{
	private:
		// ----------------------------------------------------------------
		// Events distatching.
		// ----------------------------------------------------------------
		struct WindowEventDispatcher
		{
			virtual ~WindowEventDispatcher() {}
			virtual LRESULT Dispatch(HWND const hWnd, UINT const message, WPARAM const wParam,
									 LPARAM const lParam) const = 0;
		}; // struct WindowEventDispatcher

		UI_API static auto& GetRegisteredDispatchers()
		{
			std::vector<WindowEventDispatcher const*> static registeredDispatchers;
			return registeredDispatchers;
		}
		UI_API static WORD RegisterDispatcher(WindowEventDispatcher const* const dispatcher)
		{
			auto& registeredDispatchers = GetRegisteredDispatchers();
			registeredDispatchers.push_back(dispatcher);
			assert(registeredDispatchers.size() < MAXWORD);
			return (WORD)registeredDispatchers.size() - 1;
		}
		template <typename TWindowEventDispatcher>
		UI_API static WORD RegisterDispatcher()
		{
			TWindowEventDispatcher static const dispatcher = {};
			WORD static dispatcherID = RegisterDispatcher(&dispatcher);
			return dispatcherID;
		}
		UI_API static WindowEventDispatcher const* ResolveDispatcher(WORD const id)
		{
			return GetRegisteredDispatchers()[id];
		}

		// ----------------------------------------------------------------
		// ----------------------------------------------------------------

		UI_API LRESULT static CALLBACK WindowProc(HWND const hWnd, UINT const message,
												  WPARAM const wParam, LPARAM const lParam)
		{
			auto const self = reinterpret_cast<Window*>(GetWindowLongW(hWnd, GWLP_USERDATA));
			return self != nullptr ? self->SelfWindowProc(message, wParam, lParam)
								   : DefWindowProcW(hWnd, message, wParam, lParam);
		}
		UI_API LRESULT SelfWindowProc(UINT const message, WPARAM const wParam,
									  LPARAM const lParam) const
		{
			switch (message)
			{
				case WM_COMMAND:
				{
					break;
				}

				default:
					break;
			}
			return DefWindowProcW(m_hwnd, message, wParam, lParam);
		}

		UI_API auto static Hash(wchar_t const* const string)
		{
			return static_cast<WindowWidgetHash>(std::hash<wchar_t const*>()(string));
		}

	public:
		/**
		 * \brief Initializes a new native window.
		 * \param[in] rect Rect of the window.
		 * \param[in] caption Caption of the window.
		 * \param[in] fullscreen True for fullscreen windows.
		 */
		UI_API explicit Window(Rect const& rect, wchar_t const* const caption = nullptr,
							   bool const fullscreen = false)
		{
			(void)fullscreen;

			WNDCLASSEX static windowClass;
			if (windowClass.cbSize == 0)
			{
				windowClass.cbSize = sizeof(WNDCLASSEX);
				windowClass.style = CS_HREDRAW | CS_VREDRAW;
				windowClass.lpfnWndProc = WindowProc;
				windowClass.hInstance = GetModuleHandleW(nullptr);
			//	windowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
				windowClass.lpszClassName = L"WindowClass1";
				RegisterClassEx(&windowClass);
			}

			m_hwnd = CreateWindowEx(0, windowClass.lpszClassName, caption,
									WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
									rect.x, rect.y, rect.w, rect.h, nullptr, nullptr,
									GetModuleHandleW(nullptr), nullptr);
			SetWindowLongW(m_hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			ShowWindow(m_hwnd, SW_SHOW);
		}

	public:
		// ***********************************************************************************************
		// Static controls.
		// ***********************************************************************************************

		/**
		 * \brief Adds a horizontal separator to the window.
		 * \param[in] rect Rect of the separator.
		 * \returns Native separator handle.
		 */
		UI_API WindowWidget HorizontalSeparator(Rect const& rect) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return WindowWidget(CreateWindowW(L"Static", nullptr,
											  WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, rect.x, rect.y,
											  rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		/**
		 * \brief Adds a vertical separator to the window.
		 * \param[in] rect Rect of the separator.
		 * \returns Native separator handle.
		 */
		UI_API WindowWidget VerticalSeparator(Rect const& rect) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return WindowWidget(CreateWindowW(L"Static", nullptr,
											  WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, rect.x, rect.y,
											  rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		/**
		 * \brief Adds a label to the window.
		 * \param[in] rect Rect of the label.
		 * \param[in] text Text of the label.
		 * \param[in] flags Additional label flags.
		 * \returns Native label handle.
		 */
		UI_API WindowWidget Label(Rect const& rect, wchar_t const* const text,
								  LabelFlags const flags = LabelFlags::LeftAlignment) const
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(text != nullptr);
			return WindowWidget(
				CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags),
							  rect.x, rect.y, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		/**
		 * \brief Adds an image to the window.
		 * \param[in] rect Rect of the separator.
		 * \param[in] path Path to the image file.
		 * \returns Native image handle.
		 */
		UI_API WindowWidget Image(Rect const& rect, wchar_t const* const path) const
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(path != nullptr);

			auto const handle =
				CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP, rect.x, rect.y,
							  rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr);
			if (handle != nullptr)
			{
				auto const bitmap = LoadImageW(nullptr, path, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
				if (bitmap != nullptr)
				{
					SendMessageW(handle, STM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP),
								 reinterpret_cast<LPARAM>(bitmap));
				}
			}
			return WindowWidget(handle);
		}

		// ***********************************************************************************************
		// Inputs.
		// ***********************************************************************************************

		/**
		 * \brief Adds a text editor to the window.
		 * \param[in] rect Rect of the label.
		 * \param[in] text Initial text of the editor.
		 * \param[in] flags Additional editor flags.
		 * \returns Native tex editor handle.
		 */
		UI_API WindowWidget TextEdit(Rect const& rect, wchar_t const* const text = nullptr,
									 TextEditFlags const flags = TextEditFlags::None) const
		{
			assert(rect.w != 0 && rect.h != 0);
			return WindowWidget(
				CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags),
							  rect.x, rect.y, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
		}

		// ***********************************************************************************************
		// Buttons.
		// ***********************************************************************************************

		/**
		 * \brief Adds a button to the window.
		 * \param[in] rect Rect of the button.
		 * \param[in] text Text of the button.
		 * \param[in] callback Callback function for button click.
		 * \returns Native button handle.
		 */
		UI_API WindowWidget Button(Rect const& rect, wchar_t const* const text,
								   WindowWidgetCallback&& callback)
		{
			assert(rect.w != 0 && rect.h != 0);
			assert(text != nullptr);
			(void)callback;

			auto const hash = Hash(text);
			return WindowWidget(CreateWindowW(L"Button", text, WS_VISIBLE | WS_CHILD, rect.x,
											  rect.y, rect.w, rect.h, m_hwnd,
											  reinterpret_cast<HMENU>(hash), nullptr, nullptr));
		}

		// ***********************************************************************************************
		// Direct3D 9.
		// ***********************************************************************************************

		UI_API LPDIRECT3DDEVICE9 /*WindowWidget*/ Direct3D9(Rect const& rect)
		{
			auto const handle = m_hwnd;
				/*CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE, rect.x, rect.y, rect.w,
							  rect.h, m_hwnd, nullptr, nullptr, nullptr);*/

			auto const direct3D = Direct3DCreate9(D3D_SDK_VERSION);

			LPDIRECT3DDEVICE9 device;
			D3DPRESENT_PARAMETERS presentParameters = {};
			presentParameters.Windowed = TRUE;
			presentParameters.SwapEffect = D3DSWAPEFFECT_DISCARD;
			presentParameters.hDeviceWindow = handle;
			presentParameters.BackBufferFormat = D3DFMT_X8R8G8B8;
			presentParameters.BackBufferWidth = 1280;
			presentParameters.BackBufferHeight = 720;
			presentParameters.EnableAutoDepthStencil = TRUE;
			presentParameters.AutoDepthStencilFormat = D3DFMT_D16;

			direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle,
								   D3DCREATE_SOFTWARE_VERTEXPROCESSING, &presentParameters,
								   &device);
			return device;
		}

	}; // class Window

} // namespace UI
