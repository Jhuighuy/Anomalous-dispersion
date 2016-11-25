// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "PresentationFramework.hpp"

#pragma warning(push, 0)
#include <d2d1.h>	/* Doesn't compile without this. */
#pragma warning(pop)
#include <WinCodec.h>

namespace Presentation2
{
	ADAPI bool g_IsExitRequested = false;
	ADAPI std::thread::id g_MainThreadID = std::this_thread::get_id();
	ADAPI std::thread::id g_RenderingThreadID;

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Rectangles and monitors geometry.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADINT void Monitor::GetResolution(INT& width, INT& height)
	{
		RECT static resolution = {};
		if (resolution.right == 0)
		{
			auto const desktopWindow = Utils::RuntimeCheck(GetDesktopWindow());
			Utils::RuntimeCheck(GetWindowRect(desktopWindow, &resolution));
		}
		width = resolution.right - resolution.left;
		height = resolution.bottom - resolution.top;
	}

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Basic window widget.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADINT LRESULT CALLBACK WindowProcCall(HWND const hWnd, UINT const message, WPARAM const wParam, LPARAM const lParam, UINT_PTR const, DWORD_PTR const)
	{
		auto const self = reinterpret_cast<WindowWidget*>(GetWindowLongW(hWnd, GWLP_USERDATA));
		if (self != nullptr)
		{
			return self->OnWindowProcCall(message, wParam, lParam);
		}
		return DefSubclassProc(hWnd, message, wParam, lParam);
	}

	// -----------------------
	ADAPI WindowWidget::WindowWidget(HWND const hwnd, TextSize const textSize)
		: m_Handle(hwnd)
	{
		assert(std::this_thread::get_id() == g_MainThreadID);
		assert(m_Handle != nullptr);

		Utils::RuntimeCheck(SetWindowSubclass(hwnd, &WindowProcCall, 1, 0));
		SetWindowLongW(m_Handle, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		SetTextSize(textSize);
	}

	// -----------------------
	ADAPI void WindowWidget::SetTextSize(TextSize const textSize) const
	{
		assert(m_Handle != nullptr);
		assert(textSize < TextSize::COUNT);

		struct
		{
			int const Height;
			HFONT Handle;
		} static FontsCache[static_cast<size_t>(TextSize::COUNT)] = { { 30 }, { 45 }, { 45 }, { 75 } };
		auto& font = FontsCache[static_cast<size_t>(textSize)];
		if (font.Handle == nullptr)
		{
			/* Lazily loading the system font. */
			font.Handle = Utils::RuntimeCheck(CreateFontW(Monitor::UpscaleHeight(font.Height), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
				, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial"));
		}
		SendMessageW(m_Handle, WM_SETFONT, reinterpret_cast<WPARAM>(font.Handle), TRUE);
	}
	
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Window widgets and controls.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	// -----------------------
	ADAPI LRESULT ButtonWidget::OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam)
	{
		if (message == WM_LBUTTONUP)
		{
			return 0;
		}
		return DefSubclassProc(m_Handle, message, wParam, lParam);
	}

	// -----------------------
	ADAPI LRESULT CheckboxWidget::OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam)
	{
		if (message == WM_LBUTTONUP)
		{
			return 0;
		}
		return DefSubclassProc(m_Handle, message, wParam, lParam);
	}

	// -----------------------
	ADAPI Window::Window(Rect const& rect, LPCWSTR const caption, bool const fullscreen)
	{
		WNDCLASSEX static windowClass = { };
		if (windowClass.cbSize == 0)
		{
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = DefWindowProcW;
			windowClass.hInstance = GetModuleHandleW(nullptr);
			windowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			windowClass.lpszClassName = L"Presentation2WindowClass";
			Utils::RuntimeCheck(RegisterClassExW(&windowClass));
		}

		auto static const hInstance = Utils::RuntimeCheck(GetModuleHandleW(nullptr));
		if (fullscreen)
		{
			/* Initializing fullscreen window as normal one, scaled to fullscreen. */
			m_Handle = Utils::RuntimeCheck(CreateWindowExW(0, windowClass.lpszClassName, caption
			    , WS_OVERLAPPED | WS_POPUP | WS_VISIBLE
			    , 0, 0, Monitor::GetWidth(), Monitor::GetHeight()
			    , nullptr, nullptr, hInstance, nullptr
				));
		}
		else
		{
			/* Initializing normal window. */
			m_Handle = Utils::RuntimeCheck(CreateWindowExW(0, windowClass.lpszClassName, caption
			    , WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
			    , rect.X, rect.Y, rect.Width, rect.Height
			    , nullptr, nullptr, hInstance, nullptr
				));
		}
		Hide();
	}

	// -----------------------
	ADINT LRESULT Window::OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam)
	{
		return DefSubclassProc(m_Handle, message, wParam, lParam);
	}

	// -----------------------
	ADAPI void Window::OnUpdate() 
	{
		/// @todo Move main loops here?
		IUpdatable::OnUpdate();
		for (MSG msg = {}; GetMessageW(&msg, m_Handle, 0, 0);)
		{
			TranslateMessage(&msg);
			DispatchMessageW(&msg);
		}
	}

	// ***********************************************************************************************
	// Static controls.
	// ***********************************************************************************************

	// -----------------------
	ADAPI HorizontalSeparatorPtr Window::HorizontalSeparator(Rect const& rect) const
	{
		assert(rect.Width != 0 && rect.Height != 0);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle);
	}

	// -----------------------
	ADAPI LabelPtr Window::Label(Rect const& rect, LPCWSTR const text, LabelFlags const flags, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

	// -----------------------
	ADINT static auto GetImagingFactory()
	{
		IWICImagingFactory static* imagingFactory = nullptr;
		if (imagingFactory == nullptr)
		{
			Utils::RuntimeCheckH(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER
				, IID_PPV_ARGS(&imagingFactory)));
		}
		return imagingFactory;
	}

	// -----------------------
#if SUPPORT_LOADING_FROM_FILE
	ADAPI ImagePtr Window::Image(Rect const& rect, LPCWSTR const path) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(path != nullptr);

#if !_DEBUG
		OutputDebugStringA("Loading resource from file in release version.");
#endif	// if !_DEBUG

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		if (handle != nullptr)
		{
			std::vector<BYTE> bitmapBits;
			{	/* Resizing the bitmap using some interpolation methods. */
				auto const imagingFactory = GetImagingFactory();

				IWICBitmapDecoder* bitmapDecoder = nullptr;
				Utils::RuntimeCheckH(imagingFactory->CreateDecoderFromFilename(path, nullptr
					, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &bitmapDecoder));

				IWICBitmapFrameDecode* bitmapFrameDecode = nullptr;
				Utils::RuntimeCheckH(bitmapDecoder->GetFrame(0, &bitmapFrameDecode));

				IWICBitmapScaler* bitmapScaler = nullptr;
				Utils::RuntimeCheckH(imagingFactory->CreateBitmapScaler(&bitmapScaler));
				Utils::RuntimeCheckH(bitmapScaler->Initialize(bitmapFrameDecode
					, rect.Width, rect.Height, WICBitmapInterpolationModeFant));

				/* Getting bits out of the rescaled bitmap. */
				bitmapBits.resize(rect.Width * rect.Height * 4);
				Utils::RuntimeCheckH(bitmapScaler->CopyPixels(nullptr, rect.Width * 4, bitmapBits.size(), bitmapBits.data()));

				bitmapScaler->Release();
				bitmapFrameDecode->Release();
				bitmapDecoder->Release();
			}

			auto const bitmap = CreateBitmap(rect.Width, rect.Height, 1, 32, bitmapBits.data());
			if (bitmap != nullptr)
			{
				SendMessageW(handle, STM_SETIMAGE, static_cast<WPARAM>(IMAGE_BITMAP), reinterpret_cast<LPARAM>(bitmap));
			}
		}
		return std::make_shared<WindowWidget>(handle);
	}
#endif	// if SUPPORT_LOADING_FROM_FILE
	ADAPI ImagePtr Window::Image(Rect const& rect, LoadFromMemory_t, void const* const data) const
	{
		/// @todo http://stackoverflow.com/a/2901465
		/// @todo https://msdn.microsoft.com/ru-ru/library/windows/desktop/bb773831(v=vs.85).aspx
		(void)this, rect, data;
		throw 0;
	}

	// ***********************************************************************************************
	// Inputs.
	// ***********************************************************************************************

	// -----------------------
	ADAPI TextEditPtr Window::TextEdit(Rect const& rect, LPCWSTR const text, TextEditFlags const flags, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

	// ***********************************************************************************************
	// Buttons.
	// ***********************************************************************************************

	// -----------------------
	ADAPI ButtonPtr Window::Button(Rect const& rect, LPCWSTR const text, ButtonWidgetCallback&& callback, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		return std::make_shared<ButtonWidget>(std::forward<ButtonWidgetCallback>(callback), handle, textSize);
	}

	// -----------------------
	ADAPI CheckBoxPtr Window::CheckBox(Rect const& rect, LPCWSTR const text, CheckboxWidgetCallback&& callback, bool const enabled, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE | BS_CHECKBOX | BS_MULTILINE
			, rect.X, rect.Y, rect.Width, rect.Height, m_Handle, nullptr, nullptr, nullptr));
		return std::make_shared<CheckboxWidget>(std::forward<CheckboxWidgetCallback>(callback), handle, textSize);
	}

}	// namespace Presentation2
