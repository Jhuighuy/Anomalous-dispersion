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

/* Loading the minimized version of the WinAPI. */
#ifndef WIN32_LEAN_AND_MEAN 
#define WIN32_LEAN_AND_MEAN 1
#endif	// ifndef WIN32_LEAN_AND_MEAN 
#ifndef VC_EXTRALEAN 
#define VC_EXTRALEAN 1
#endif	// ifndef VC_EXTRALEAN 
#ifndef NOMINMAX 
#define NOMINMAX 1
#endif	// ifndef NOMINMAX 
#include <Windows.h>
#include <CommCtrl.h>
#include <D3D9.h>

#include <memory>
#include <vector>
#include <thread>
#include <cassert>
#include <algorithm>
#include <functional>

#define ADAPI
#define ADINT
#define ADINL __forceinline
#pragma warning(disable : 4714) // function 'function' marked as __forceinline not inlined

/* We want to have resources being compiled into the exe */
#define SUPPORT_LOADING_FROM_FILE /*_DEBUG*/1

namespace Presentation2
{
	enum LoadFromMemory_t { LoadFromMemory };

	ADAPI bool extern g_IsExitRequested;
	ADAPI extern std::thread::id g_MainThreadID;
	ADAPI extern std::thread::id g_RenderingThreadID;

	struct INonCopyable
	{
	protected:
		ADINT INonCopyable() = default;
		ADINT INonCopyable(INonCopyable&&) = delete;
		ADINT INonCopyable(INonCopyable const&) = delete;
		ADINT INonCopyable& operator=(INonCopyable&&) = delete;
		ADINT INonCopyable& operator=(INonCopyable const&) = delete;
	};	// struct INonCopyable

	struct IUpdatable : INonCopyable
	{
		ADINT virtual ~IUpdatable() = default;
		ADAPI virtual void OnUpdate() = 0
		{
		//	assert(std::this_thread::get_id() == g_MainThreadID);
		}
	};	// struct IUpdatable

	struct IRenderable : public IUpdatable
	{
		ADAPI virtual void OnRender() const = 0
		{
		//	assert(std::this_thread::get_id() == g_RenderingThreadID);
		}
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

	/***************
	 * Monitor resolution API. */
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
		ADINL Rect(UpperLeftPivot_t, NoScaling_t, INT const x, INT const y, INT const w, INT const h)
			: X(x), Y(y), Width(w), Height(h)
		{}
		ADINL Rect(UpperRightPivot_t, NoScaling_t, INT const x, INT const y, INT const w, INT const h)
			: X(x - w), Y(y), Width(w), Height(h)
		{}
		ADINL Rect(LowerLeftPivot_t, NoScaling_t, INT const x, INT const y, INT const w, INT const h)
			: X(x), Y(y - h), Width(w), Height(h)
		{}
		ADINL Rect(LowerRightPivot_t, NoScaling_t, INT const x, INT const y, INT const w, INT const h)
			: X(x - w), Y(y - h), Width(w), Height(h)
		{}
		ADINL Rect(CenterPivot_t, NoScaling_t, INT const x, INT const y, INT const w, INT const h)
			: X(x - w / 2), Y(y - h / 2), Width(w), Height(h)
		{}
		// -----------------------
		ADINL Rect(UpperLeftPivot_t, INT const x, INT const y, INT const w, INT const h)
			: Rect(UpperLeftPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleY(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(UpperRightPivot_t, INT const x, INT const y, INT const w, INT const h)
			: Rect(UpperRightPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleY(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(LowerLeftPivot_t, INT const x, INT const y, INT const w, INT const h)
			: Rect(LowerLeftPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleY(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(LowerRightPivot_t, INT const x, INT const y, INT const w, INT const h)
			: Rect(LowerRightPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleY(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		ADINL Rect(CenterPivot_t, INT const x, INT const y, INT const w, INT const h)
			: Rect(CenterPivot, NoScaling, Monitor::UpscaleX(x), Monitor::UpscaleY(y), Monitor::UpscaleWidth(w), Monitor::UpscaleHeight(h))
		{}
		// -----------------------
		ADINL Rect(INT const x, INT const y, INT const w, INT const h)
			: Rect(CenterPivot, x, y, w, h)
		{}
	};	// struct Rect 

	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //
	// Basic window widget.
	// XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX //

	using WindowWidgetPtr = std::shared_ptr<class WindowWidget>;
	
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
	public:
		HWND m_Handle;

	public:
		// -----------------------
		ADINL WindowWidget() : m_Handle(nullptr) {}
		ADAPI explicit WindowWidget(HWND const hwnd, TextSize const textSize = TextSize::Default);
		// -----------------------
		ADAPI virtual ~WindowWidget()
		{
			DestroyWindow(m_Handle);
		}

		// -----------------------
		ADAPI virtual LRESULT OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam)
		{
			return DefSubclassProc(m_Handle, message, wParam, lParam);
		}

		// -----------------------
		ADAPI void SetTextSize(TextSize const textSize = TextSize::Default) const;

		// -----------------------
		ADINL void SetText(LPCWSTR const text) const
		{
			assert(m_Handle != nullptr);
			SetWindowTextW(m_Handle, text);
		}

		// -----------------------
		ADINL void Show(bool const show = true) const
		{
			assert(m_Handle != nullptr);
			ShowWindow(m_Handle, show ? SW_SHOW : SW_HIDE);
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
		CenterAlignment = SS_CENTER,
		RightAlignment = SS_RIGHT,
	}; // enum LabelFlags

	using HorizontalSeparatorPtr = WindowWidgetPtr;
	using LabelPtr = WindowWidgetPtr;
	using ImagePtr = WindowWidgetPtr;

	enum class TextEditFlags : DWORD
	{
		None = 0,
		CenterAlignment = ES_CENTER,
		LeftAlignment = ES_LEFT,
		MultiLine = ES_MULTILINE,
		Numbers = ES_NUMBER,
		ReadOnly = ES_READONLY,
		RightAlignment = ES_RIGHT,
	}; // enum class TextEditFlags

	using TextEditPtr = WindowWidgetPtr;

	using ButtonWidgetCallback = std::function<void()>;
	using CheckboxWidgetCallback = std::function<void(bool)>;

	/***************
	 * Simple wrapper around raw WinAPI buttons. */
	class ButtonWidget final : public WindowWidget
	{
	private:
		ButtonWidgetCallback m_Callback;

	public:
		// -----------------------
		ADINL ButtonWidget(ButtonWidgetCallback&& callback, HWND const hwnd, TextSize const textSize = TextSize::Default)
			: WindowWidget(hwnd, textSize), m_Callback(callback) {}
		ADAPI LRESULT OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam) override final;
	};	// class ButtonWidget
	using ButtonPtr = std::shared_ptr<ButtonWidget>;

	/***************
	 * Simple wrapper around raw WinAPI checkboxes. */
	class CheckboxWidget final : public WindowWidget
	{
	private:
		CheckboxWidgetCallback m_Callback;

	public:
		// -----------------------
		ADINL CheckboxWidget(CheckboxWidgetCallback&& callback, HWND const hwnd, TextSize const textSize = TextSize::Default)
			: WindowWidget(hwnd, textSize), m_Callback(callback) {}
		ADAPI LRESULT OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam) override final;
	};	// class ButtonWidget
	using CheckBoxPtr = std::shared_ptr<CheckboxWidget>;

	/*************** 
	 * Simple wrapper around raw WinAPI Direct3D 9 device handle. */
	class GraphicsWidget : public WindowWidget, public IRenderable
	{
	public:
		IDirect3DDevice9* const m_Device;
	public:
		ADINL GraphicsWidget(HWND const hwnd, IDirect3DDevice9* const device) 
			: WindowWidget(hwnd), m_Device(device) {}
	};	// class GraphicsWidget
	using GraphicsWidgetPtr = std::shared_ptr<class GraphicsWidget>;

	/*************** 
	 * Simple wrapper around raw WinAPI window. */
	class Window : public WindowWidget, public IUpdatable
	{
	public:
		// -----------------------
		ADAPI explicit Window(Rect const& rect, LPCWSTR const caption = nullptr, bool const fullscreen = false);

	public:
		// -----------------------
		ADAPI LRESULT OnWindowProcCall(UINT const message, WPARAM const wParam, LPARAM const lParam) override final;
		ADAPI void OnUpdate() override;

		// ***********************************************************************************************
		// Static controls.
		// ***********************************************************************************************

		// -----------------------
		ADAPI HorizontalSeparatorPtr HorizontalSeparator(Rect const& rect) const;

		// -----------------------
		ADAPI LabelPtr Label(Rect const& rect, LPCWSTR const text, LabelFlags const flags = LabelFlags::LeftAlignment, TextSize const textSize = TextSize::Default) const;

		// -----------------------
#if SUPPORT_LOADING_FROM_FILE
		ADAPI ImagePtr Image(Rect const& rect, LPCWSTR const path) const;
#endif	// if SUPPORT_LOADING_FROM_FILE
		ADAPI ImagePtr Image(Rect const& rect, LoadFromMemory_t, void const* const data) const;

		// ***********************************************************************************************
		// Inputs.
		// ***********************************************************************************************

		// -----------------------
		ADAPI TextEditPtr TextEdit(Rect const& rect, LPCWSTR const text = nullptr, TextEditFlags const flags = TextEditFlags::None, TextSize const textSize = TextSize::Default) const;

		// ***********************************************************************************************
		// Buttons.
		// ***********************************************************************************************

		// -----------------------
		ADAPI ButtonPtr Button(Rect const& rect, LPCWSTR const text, ButtonWidgetCallback&& callback, TextSize const textSize = TextSize::Default) const;

		// -----------------------
		ADAPI CheckBoxPtr CheckBox(Rect const& rect, LPCWSTR const text, CheckboxWidgetCallback&& callback, bool const enabled = false, TextSize const textSize = TextSize::Default) const;

		// ***********************************************************************************************
		// Graphics.
		// ***********************************************************************************************

		// -----------------------
		template<typename TGraphicsWidget = GraphicsWidget, typename... TArgs>
		ADAPI std::shared_ptr<TGraphicsWidget> Graphics(Rect const& rect, TArgs&&... args) const;

	};	// class Window
	using WindowPtr = std::shared_ptr<class Window>;

}	// namespace Presentation2

#include "PresentationFramework.inl"
