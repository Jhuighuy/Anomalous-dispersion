// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#include "PresentationFramework.hpp"

#pragma warning(push, 0)
#include <d2d1.h>	/* Doesn't compile without this. */
#pragma warning(pop)
#include <WinCodec.h>
#pragma comment(lib, "WindowsCodecs.lib")

namespace Presentation2
{
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
	ADAPI WindowWidget::WindowWidget(HWND const hwnd, TextSize const textSize)
		: m_Hwnd(hwnd)
	{
		assert(std::this_thread::get_id() == g_MainThreadID);
		assert(m_Hwnd != nullptr);

		SetTextSize(textSize);
	}

	// -----------------------
	ADAPI void WindowWidget::SetTextSize(TextSize const textSize) const
	{
		assert(m_Hwnd != nullptr);
		assert(textSize < TextSize::COUNT);

		struct
		{
			int const Height;
			HFONT Handle;
		} static FontsCache[static_cast<size_t>(TextSize::COUNT)] = { { 20 }, { 30 }, { 40 }, { 75 } };
		auto& font = FontsCache[static_cast<size_t>(textSize)];
		if (font.Handle == nullptr)
		{
			/* Lazily loading the system font. */
			font.Handle = Utils::RuntimeCheck(CreateFontW(Monitor::UpscaleHeight(font.Height), 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE
				, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Consolas"));
		}
		SendMessageW(m_Hwnd, WM_SETFONT, reinterpret_cast<WPARAM>(font.Handle), TRUE);
	}
	
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Window widgets and controls.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	ADAPI bool g_IsExitRequested = false;

	// -----------------------
	ADINT LRESULT CALLBACK Window::WindowProc(HWND const hWnd, UINT const message, WPARAM const wParam, LPARAM const lParam)
	{
		if (message == WM_CLOSE)
		{
			PostQuitMessage(0);
			g_IsExitRequested = true;
		}
		else if (message == WM_COMMAND)
		{
			/* We need to process messages from buttons only. */
			auto const self = reinterpret_cast<Window*>(GetWindowLongW(hWnd, GWLP_USERDATA));
			if (self != nullptr)
			{
				auto const controlID = static_cast<WORD>(LOWORD(wParam));
				if (self->CheckID(controlID))
				{
					auto const& controlCallback = self->m_Callbacks[UnwrapID(controlID)];
					auto const isControlChecked = IsDlgButtonChecked(self->m_Hwnd, controlID);
					CheckDlgButton(self->m_Hwnd, controlID, !isControlChecked ? BST_CHECKED : BST_UNCHECKED);
					controlCallback(!isControlChecked);
				}
			}
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	// -----------------------
	ADAPI Window::Window(Rect const& rect, wchar_t const* const caption, bool const fullscreen)
	{
		WNDCLASSEX static windowClass = { };
		if (windowClass.cbSize == 0)
		{
			windowClass.cbSize = sizeof(WNDCLASSEX);
			windowClass.style = CS_HREDRAW | CS_VREDRAW;
			windowClass.lpfnWndProc = WindowProc;
			windowClass.hInstance = GetModuleHandleW(nullptr);
			windowClass.hbrBackground = GetSysColorBrush(COLOR_3DFACE);
			windowClass.lpszClassName = L"Presentation2WindowClass";
			Utils::RuntimeCheck(RegisterClassExW(&windowClass));
		}

		auto static const hInstance = Utils::RuntimeCheck(GetModuleHandleW(nullptr));
		if (fullscreen)
		{
			/* Initializing fullscreen window as normal one, scaled to fullscreen. */
			m_Hwnd = Utils::RuntimeCheck(CreateWindowExW(0, windowClass.lpszClassName, caption
			    , WS_POPUP | WS_VISIBLE | WS_OVERLAPPED
			    , 0, 0, Monitor::GetWidth(), Monitor::GetHeight()
			    , nullptr, nullptr, hInstance, nullptr
				));
		}
		else
		{
			/* Initializing normal window. */
			m_Hwnd = Utils::RuntimeCheck(CreateWindowExW(0, windowClass.lpszClassName, caption
			    , WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX
			    , rect.X, rect.Y, rect.Width, rect.Height
			    , nullptr, nullptr, hInstance, nullptr
				));
		}
		SetWindowLongW(m_Hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
		Hide();
	}

	// -----------------------
	ADAPI void Window::Update() const
	{
		/// @todo Move main loops here?
	}

	// ***********************************************************************************************
	// Static controls.
	// ***********************************************************************************************

	// -----------------------
	ADAPI HorizontalSeparatorPtr Window::HorizontalSeparator(Rect const& rect) const
	{
		assert(rect.Width != 0 && rect.Height != 0);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_ETCHEDHORZ
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle);
	}

	// -----------------------
	ADAPI LabelPtr Window::Label(Rect const& rect, wchar_t const* const text, LabelFlags const flags, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

	// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI ImagePtr Window::Image(Rect const& rect, wchar_t const* const path) const
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(path != nullptr);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Static", nullptr, WS_CHILD | WS_VISIBLE | SS_BITMAP
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr));
		if (handle != nullptr)
		{
			std::vector<BYTE> bitmapBits;
			{	/* Resizing the bitmap using some interpolation methods. */
				IWICImagingFactory static* imagingFactory = nullptr;
				if (imagingFactory == nullptr)
				{
					Utils::RuntimeCheckH(CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER
						, IID_PPV_ARGS(&imagingFactory)));
				}

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
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
	ADAPI ImagePtr Window::Image(Rect const& rect, LoadFromMemory_t, void const* const data) const
	{
		// http://stackoverflow.com/a/2901465
		(void)this, rect, data;
		throw 0;
	}

	// ***********************************************************************************************
	// Inputs.
	// ***********************************************************************************************

	// -----------------------
	ADAPI TextEditPtr Window::TextEdit(Rect const& rect, wchar_t const* const text, TextEditFlags const flags, TextSize const textSize) const
	{
		assert(rect.Width != 0 && rect.Height != 0);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Edit", text, WS_CHILD | WS_VISIBLE | static_cast<DWORD>(flags)
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, nullptr, nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

	// ***********************************************************************************************
	// Buttons.
	// ***********************************************************************************************

	// -----------------------
	ADAPI ButtonPtr Window::Button(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, TextSize const textSize)
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const id = GenID();
		m_Callbacks.push_back(callback);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, reinterpret_cast<HMENU>(id), nullptr, nullptr));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

	// -----------------------
	ADAPI CheckBoxPtr Window::CheckBox(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, bool const enabled, TextSize const textSize)
	{
		assert(rect.Width != 0 && rect.Height != 0);
		assert(text != nullptr);

		auto const id = GenID();
		m_Callbacks.push_back(callback);

		auto const handle = Utils::RuntimeCheck(CreateWindowW(L"Button", text, WS_CHILD | WS_VISIBLE | BS_CHECKBOX
			, rect.X, rect.Y, rect.Width, rect.Height, m_Hwnd, reinterpret_cast<HMENU>(id), nullptr, nullptr));
		Utils::RuntimeCheck(CheckDlgButton(m_Hwnd, id, enabled ? BST_CHECKED : BST_UNCHECKED));
		return std::make_shared<WindowWidget>(handle, textSize);
	}

}	// namespace Presentation2
