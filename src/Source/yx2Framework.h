// Copyright (C) 2016 Butakov Oleg, Plaxin Gleb.
// Computational Methods @ Computational Maathematics & Cybernetics, MSU.

#pragma once
#ifndef _WIN32
#	error YX2 engine supports Windows-based platforms only.
#endif	// ifndef _WIN32

#include <memory>
#include <string>

#include <d3d9.h>
#include <assert.h>
#include <Windows.h>

#pragma warning(push, 0)
#include <glm/glm.hpp>
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtx/rotate_vector.hpp>
#pragma warning(pop)

#define YX2_API

namespace yx2
{
	using glm::vec2;

	struct NonCopyable
	{
		NonCopyable() = default;
		NonCopyable(NonCopyable&&) = delete;
		NonCopyable(NonCopyable const&) = delete;
		NonCopyable& operator= (NonCopyable&) = delete;
		NonCopyable& operator= (NonCopyable const&) = delete;
	};	// struct non_copyable

	template<typename T>
	static void Validate(T const& what)
	{
		(void)what;
		assert(what);
	}

	namespace framework
	{
		struct rect
		{
		public:
			int const x;
			int const y;
			unsigned const w;
			unsigned const h;

			rect() : x(0), y(0), w(0), h(0) {}
			rect(int const x, int const y, unsigned const w, unsigned const h) : x(x), y(y), w(w), h(h)
			{
			}
		}; // struct Rect

		/**
		 * \todo Implement Goog-like architecture.
		 * \see https://www.facepunch.com/threads/1108886-Goop-OOP-WinAPI-GUI-Wrapper
		 */
		struct widget : public NonCopyable
		{
		private:
			HWND m_WidgetHandle;

		protected:
			YX2_API widget(LPCWSTR const className, LPCWSTR const widgetName, DWORD const style, rect const& rect, HMENU const menu)
				: m_WidgetHandle(CreateWindowExW(0, className, widgetName, style, rect.x, rect.y, rect.w, rect.h, nullptr, nullptr, nullptr, nullptr))
			{
			}

		public:
			// ----------------------------------------------------------------
#pragma region

			/**
			 * \brief Checks whether this widget is enabled.
			 * \returns Whether this widget is enabled.
			 */
			YX2_API bool IsEnabled() const
			{
				return IsWindowEnabled(m_WidgetHandle);
			}

			/**
			 * \brief Enables this widget.
			 */
			YX2_API void EnableWidget() const
			{
				Validate(EnableWindow(m_WidgetHandle, TRUE));
			}

			/**
			 * \brief Disables this widget.
			 */
			YX2_API void DisableWidget() const
			{
				Validate(EnableWindow(m_WidgetHandle, FALSE));
			}

			/**
			 * \brief Enables rendering for this widget.
			 */
			YX2_API void ShowWidget() const
			{
				Validate(ShowWindow(m_WidgetHandle, SW_SHOW));
			}

			/**
			* \brief Disables rendering for this widget.
			*/
			YX2_API void HideWidget() const
			{
				Validate(ShowWindow(m_WidgetHandle, SW_HIDE));
			}

#pragma endregion 
			// ----------------------------------------------------------------


		};	// struct widget

		struct window
		{
		};	// struct window

		struct D3DWidget : widget
		{
		public:
			LPDIRECT3DDEVICE9 m_device;

		};	// struct d3d_widget 

	}	// namespace framework
}	// namespace yx2
