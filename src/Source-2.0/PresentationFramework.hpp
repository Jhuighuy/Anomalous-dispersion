// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX
// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Mathematics & Cybernetics, MSU.
// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX

#pragma once
#ifndef _WIN32
#error This application supports Windows-based platforms only.
#endif // ifndef _WIN32
#ifdef _WINDOWS_
#error Please, include <Windows.h> before this header.
#endif	// ifdef _WINDOWS_

#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif	// ifndef WIN32_LEAN_AND_MEAN 
#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN 1
#endif	// ifndef VC_EXTRALEAN 
#ifndef NOMINMAX 
#define NOMINMAX 1
#endif	// ifndef NOMINMAX 
#include <d3d9.h>

#include <memory>
#include <vector>
#include <atomic>
#include <cassert>
#include <algorithm>
#include <functional>

#define ADAPI
#define ADINL
#define ADINT
#define SUPPORT_LOADING_FROM_FILE !_DEBUG
#pragma warning(disable : 4505)

namespace Presentation2
{
	enum LoadFromMemory_t { LoadFromMemory };

	struct INonCopyable
	{
		ADINT INonCopyable() = default;
		ADINT INonCopyable(INonCopyable&&) = delete;
		ADINT INonCopyable(INonCopyable const&) = delete;
		ADINT INonCopyable& operator=(INonCopyable&&) = delete;
		ADINT INonCopyable& operator=(INonCopyable const&) = delete;
	};	// struct INonCopyable

	struct IUpdatable : INonCopyable
	{
		ADINT virtual ~IUpdatable() = default;
		ADAPI virtual void Update() const = 0;
	};	// struct IUpdatable

	struct IRenderable : public IUpdatable
	{
		ADAPI virtual void Render() const = 0;
	};	// struct IRenderable

	namespace Utils
	{
		template<typename T>
		ADINL static T Clamp(T const value, T const min, T const max)
		{
			return value < min ? min : value > max ? max : value;
		}

		template<typename T>
		ADINL static void RuntimeCheck(T const value)
		{
			if (!value)
			{
				throw std::runtime_error("Fuck this shit.");
			}
		}
		ADINL static void RuntimeCheckH(HRESULT const value)
		{
			RuntimeCheck(SUCCEEDED(value));
		}
		template<typename T>
		ADINL static T* RuntimeCheck(T* const value)
		{
			Utils::RuntimeCheck(value != nullptr && value != INVALID_HANDLE_VALUE);
			return value;
		}

	}	// namespace Utils

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

		ADINT static void GetResolution(INT& width, INT& height);

	public:
		// -----------------------
		ADINT static auto GetWidth()
		{
			INT w, h;
			GetResolution(w, h);
			return w;
		}
		ADINT static auto GetHeight()
		{
			INT w, h;
			GetResolution(w, h);
			return h;
		}

		// -----------------------
		ADINL static auto UpscaleX(INT const width)
		{
			return width * GetWidth() / DEFAULT_DESKTOP_WIDTH;
		}
		ADINL static auto UpscaleY(INT const height)
		{
			return height * GetHeight() / DEFAULT_DESKTOP_HEIGHT;
		}

		// -----------------------
		ADINL static auto UpscaleWidth(INT const width)
		{
			return std::max(1, UpscaleX(width));
		}
		ADINL static auto UpscaleHeight(INT const height)
		{
			return std::max(1, UpscaleY(height));
		}
	};	// class Monitor

	enum CenterPivot_t { CenterPivot };
	enum UpperLeftPivot_t { UpperLeftPivot };
	enum UpperRightPivot_t { UpperRightPivot };
	enum LowerLeftPivot_t { LowerLeftPivot };
	enum LowerRightPivot_t { LowerRightPivot };
	
	enum NoScaling_t { NoScaling };

	/*************** 
	 * Scalable window rectangle. */
	struct Rect final
	{
		INT X, Y;
		INT Width, Height;

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

	using WindowWidgetPtr = std::shared_ptr<class WindowWidget>;
	ADAPI bool extern g_IsExitting;
	
	enum class TextSize : DWORD
	{
		Default,
		NotSoLarge,
		Large,
		VeryLarge,
		COUNT,
	};	// enum class TextSize

	/***************
	 * Simple wrapper around raw WinAPI HWND handle. */
	class WindowWidget
	{
	protected:
		HWND m_Hwnd;

	public:
		// -----------------------
		ADINL WindowWidget() : m_Hwnd(nullptr) {}
		ADAPI explicit WindowWidget(HWND const hwnd, TextSize const textSize = TextSize::Default)
			: m_Hwnd(hwnd)
		{
			SetTextSize(textSize);
		}
		// -----------------------
		ADAPI virtual ~WindowWidget()
		{
			DestroyWindow(m_Hwnd);
		}

	public:
		// -----------------------
		ADAPI void SetTextSize(TextSize const textSize = TextSize::Default) const;

		// -----------------------
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

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Window widgets and controls.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	enum class LabelFlags : DWORD
	{
		LeftAlignment = SS_LEFT,
		LeftAlignmentNoWordWrap = SS_LEFTNOWORDWRAP,
		CenterAlignment = SS_CENTER,
		RightAlignment = SS_RIGHT,
	}; // enum LabelFlags

	using HorizontalSeparatorPtr = WindowWidgetPtr;
	using LabelPtr = WindowWidgetPtr;
	using ImagePtr = WindowWidgetPtr;

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

	using TextEditPtr = WindowWidgetPtr;

	using ButtonPtr = WindowWidgetPtr;
	using CheckBoxPtr = WindowWidgetPtr;

	using D3DWidgetPtr = std::shared_ptr<class D3DWidget>;
	using WindowPtr = std::shared_ptr<class Window>;
	using WindowWidgetCallback = std::function<void(long)>;

	/*************** 
	 * Simple wrapper around raw WinAPI Direct3D 9 device handle. */
	class D3DWidget : public WindowWidget, public IRenderable
	{
	public:
		IDirect3DDevice9* const m_Device;
		explicit D3DWidget(HWND const hwnd, IDirect3DDevice9* const device) : WindowWidget(hwnd), m_Device(device) {}

	};	// class D3DWidget

	/*************** 
	 * Simple wrapper around raw WinAPI window. */
	class Window : public WindowWidget, public IUpdatable
	{
	private:
		std::vector<WindowWidgetCallback> m_Callbacks;

		// -----------------------
		ADINT LRESULT static CALLBACK WindowProc(HWND const hWnd, UINT const message, WPARAM const wParam, LPARAM const lParam);
		ADINT WORD GenID() const
		{
			return static_cast<WORD>(m_Callbacks.size());
		}

	public:
		// -----------------------
		ADAPI explicit Window(Rect const& rect, wchar_t const* const caption = nullptr, bool const fullscreen = false);

		// -----------------------
		ADAPI void Update() const override;

		// ***********************************************************************************************
		// Static controls.
		// ***********************************************************************************************

		// -----------------------
		ADAPI HorizontalSeparatorPtr HorizontalSeparator(Rect const& rect) const;

		// -----------------------
		ADAPI LabelPtr Label(Rect const& rect, wchar_t const* const text, LabelFlags const flags = LabelFlags::LeftAlignment, TextSize const textSize = TextSize::Default) const;

		// -----------------------
#ifdef SUPPORT_LOADING_FROM_FILE
		ADAPI ImagePtr Image(Rect const& rect, wchar_t const* const path) const;
#endif	// ifdef SUPPORT_LOADING_FROM_FILE
		ADAPI ImagePtr Image(Rect const& rect, LoadFromMemory_t, void const* const data) const;

		// ***********************************************************************************************
		// Inputs.
		// ***********************************************************************************************

		// -----------------------
		ADAPI TextEditPtr TextEdit(Rect const& rect, wchar_t const* const text = nullptr, TextEditFlags const flags = TextEditFlags::None, TextSize const textSize = TextSize::Default) const;

		// ***********************************************************************************************
		// Buttons.
		// ***********************************************************************************************

		// -----------------------
		ADAPI ButtonPtr Button(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, TextSize const textSize = TextSize::Default);

		// -----------------------
		ADAPI CheckBoxPtr CheckBox(Rect const& rect, wchar_t const* const text, WindowWidgetCallback&& callback, bool const enabled = false, TextSize const textSize = TextSize::Default);

		// ***********************************************************************************************
		// Direct3D 9.
		// ***********************************************************************************************

		// -----------------------
		template<typename D3DWidget_t = D3DWidget, typename D3DWidgetPtr_t = std::shared_ptr<D3DWidget_t>>
		ADAPI D3DWidgetPtr_t Direct3D9(Rect const& rect) const;

	};	// class Window

}	// namespace Presentation2

#include "PresentationFramework.inl"
