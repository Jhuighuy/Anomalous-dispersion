// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Maathematics & Cybernetics, MSU.

#pragma once
#ifndef _WIN32
#error YX2 engine supports Windows-based platforms only.
#endif // ifndef _WIN32

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <Windows.h>
#include <assert.h>
#include <d3d9.h>
#include <wtypes.h>
#include <locale.h>


#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

#define YX2_API

#define STANDART_DESKTOP_WIDTH 1920
#define STANDART_DESKTOP_HEIGHT 1080

namespace yx2
{
	using glm::vec2;

	struct NonCopyable
	{
		NonCopyable() = default;
		NonCopyable(NonCopyable&&) = delete;
		NonCopyable(NonCopyable const&) = delete;
		NonCopyable& operator=(NonCopyable const&) = delete;
	}; // struct non_copyable

	namespace framework
	{
		void GetDesktopResolution(int& horizontal, int& vertical)
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
			unsigned  w;
			unsigned  h;

			Rect() : x(0), y(0), w(0), h(0) {}
			//Rect(int const x, int const y, unsigned const w, unsigned const h) : x(x), y(y), w(w), h(h) {}

			Rect(int const x1, int const y1, unsigned const w1, unsigned const h1)
			{
				int horizontal, vertical;
				GetDesktopResolution(horizontal, vertical);
				x = round(x1 * horizontal / STANDART_DESKTOP_WIDTH);
				y = round(y1 * vertical / STANDART_DESKTOP_HEIGHT);
				w = round(w1 * horizontal / STANDART_DESKTOP_WIDTH);
				h = round(h1 * vertical / STANDART_DESKTOP_HEIGHT);

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
			YX2_API std::wstring GetText() const
			{
				std::wstring text(GetWindowTextLengthW(m_hwnd) + 1, L'\0');
				GetWindowTextW(m_hwnd, &text[0], text.size() + 1);
				return text;
			}
			YX2_API void SetText(wchar_t const* const text) const { SetWindowTextW(m_hwnd, text); }

		}; // class WindowWidget

		class D3DWidget : WindowWidget
		{
		public:
			IDirect3DDevice9* m_Device;
			explicit D3DWidget(HWND const hwnd, IDirect3DDevice9* const device) : WindowWidget(hwnd), m_Device(device) {}
		};

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

			YX2_API static auto& GetRegisteredDispatchers()
			{
				std::vector<WindowEventDispatcher const*> static registeredDispatchers;
				return registeredDispatchers;
			}
			YX2_API static WORD RegisterDispatcher(WindowEventDispatcher const* const dispatcher)
			{
				auto& registeredDispatchers = GetRegisteredDispatchers();
				registeredDispatchers.push_back(dispatcher);
				assert(registeredDispatchers.size() < MAXWORD);
				return (WORD)registeredDispatchers.size() - 1;
			}
			template <typename TWindowEventDispatcher>
			YX2_API static WORD RegisterDispatcher()
			{
				TWindowEventDispatcher static const dispatcher = {};
				WORD static dispatcherID = RegisterDispatcher(&dispatcher);
				return dispatcherID;
			}
			YX2_API static WindowEventDispatcher const* ResolveDispatcher(WORD const id)
			{
				return GetRegisteredDispatchers()[id];
			}

			// ----------------------------------------------------------------
			// ----------------------------------------------------------------

			YX2_API LRESULT static CALLBACK WindowProc(HWND const hWnd, UINT const message, WPARAM const wParam,
													  LPARAM const lParam)
			{
				auto const self = reinterpret_cast<Window*>(GetWindowLongW(hWnd, GWLP_USERDATA));
				return self != nullptr ? self->SelfWindowProc(message, wParam, lParam)
									   : DefWindowProcW(hWnd, message, wParam, lParam);
			}
			YX2_API LRESULT SelfWindowProc(UINT const message, WPARAM const wParam, LPARAM const lParam) const
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

			YX2_API auto static Hash(wchar_t const* const string)
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
			YX2_API explicit Window(Rect const& rect, wchar_t const* const caption = nullptr,
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
										WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX, rect.x, rect.y,
										rect.w, rect.h, nullptr, nullptr, GetModuleHandleW(nullptr), nullptr);
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
			YX2_API WindowWidget HorizontalSeparator(Rect const& rect) const
			{
				assert(rect.w != 0 && rect.h != 0);
				return WindowWidget(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ, rect.x,
												  rect.y, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
			}

			/**
			* \brief Adds a vertical separator to the window.
			* \param[in] rect Rect of the separator.
			* \returns Native separator handle.
			*/
			YX2_API WindowWidget VerticalSeparator(Rect const& rect) const
			{
				assert(rect.w != 0 && rect.h != 0);
				return WindowWidget(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDVERT, rect.x,
												  rect.y, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
			}

			/**
			* \brief Adds a label to the window.
			* \param[in] rect Rect of the label.
			* \param[in] text Text of the label.
			* \param[in] flags Additional label flags.
			* \returns Native label handle.
			*/
			YX2_API WindowWidget Label(Rect const& rect, wchar_t const* const text,
									  LabelFlags const flags = LabelFlags::LeftAlignment) const
			{
				assert(rect.w != 0 && rect.h != 0);
				assert(text != nullptr);
				return WindowWidget(CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags),
												  rect.x, rect.y, rect.w, rect.h, m_hwnd, nullptr, nullptr, nullptr));
			}

			/**
			* \brief Adds an image to the window.
			* \param[in] rect Rect of the separator.
			* \param[in] path Path to the image file.
			* \returns Native image handle.
			*/
			YX2_API WindowWidget Image(Rect const& rect, wchar_t const* const path) const
			{
				assert(rect.w != 0 && rect.h != 0);
				assert(path != nullptr);

				auto const handle = CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP, rect.x, rect.y,
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
			YX2_API WindowWidget TextEdit(Rect const& rect, wchar_t const* const text = nullptr,
										 TextEditFlags const flags = TextEditFlags::None) const
			{
				assert(rect.w != 0 && rect.h != 0);
				return WindowWidget(CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags),
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
			YX2_API WindowWidget Button(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback)
			{
				assert(rect.w != 0 && rect.h != 0);
				assert(text != nullptr);
				(void)callback;

				auto const hash = Hash(text);
				return WindowWidget(CreateWindowW(L"Button", text, WS_VISIBLE | WS_CHILD, rect.x, rect.y, rect.w,
												  rect.h, m_hwnd, reinterpret_cast<HMENU>(hash), nullptr, nullptr));
			}

			// ***********************************************************************************************
			// Direct3D 9.
			// ***********************************************************************************************

			template<typename TD3DWidget = D3DWidget>
			YX2_API TD3DWidget* Direct3D9(Rect const& rect) const
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

				direct3D->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, handle, D3DCREATE_SOFTWARE_VERTEXPROCESSING,
									   &presentParameters, &device);
				return new TD3DWidget(handle, device);
			}

		}; // class Window

	} // namespace framework
} // namespace yx2
