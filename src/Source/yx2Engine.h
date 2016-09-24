#pragma once

#include "yx2Framework.h"

#include <type_traits>
#include <vector>

#define _USE_MATH_DEFINES 1
#define GLM_FORCE_LEFT_HANDED 1
#include <math.h>

#pragma warning(push, 0)
#include <glm/gtx/euler_angles.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <glm/gtx/transform.hpp>
#pragma warning(pop)

#define DXM_PI float(M_PI)

namespace yx2
{
	using glm::vec3;
	using glm::vec4;
	using glm::mat4;

	using color32 = D3DCOLOR;
	auto static const color_black = D3DCOLOR_RGBA(0x00, 0x00, 0x00, 0xFF);
	auto static const color_white = D3DCOLOR_RGBA(0xFF, 0xFF, 0xFF, 0xFF);

	namespace engine
	{
		/**
		 * \brief Extracts a Direct3D matrix pointer from GLM matrix.
		 */
		YX2_API static D3DMATRIX const* ToD3D(mat4 const& matrix)
		{
			return reinterpret_cast<D3DMATRIX const*>(&matrix[0][0]);
		}

		YX2_API static float clampf(float value, float min, float max)
		{
			return value < min ? min : value > max ? max : value;
		}

		// ***********************************************************************************************
		// Runtime management.
		// ***********************************************************************************************

		class Runtime : public framework::D3DWidget
		{
		public:
			YX2_API Runtime(HWND const hwnd, IDirect3DDevice9* const device) : D3DWidget(hwnd, device)
			{
				m_Device->SetRenderState(D3DRS_ZENABLE, TRUE);
				m_Device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);

				m_Device->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				m_Device->SetRenderState(D3DRS_BLENDOP, D3DBLENDOP_ADD);
				m_Device->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
				m_Device->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ONE);

				m_Device->SetRenderState(D3DRS_LIGHTING, FALSE);
				m_Device->SetRenderState(D3DRS_AMBIENT, D3DCOLOR_XRGB(50, 50, 50));
			}

			YX2_API void BeginRender() const
			{
				m_Device->Clear(0, nullptr, D3DCLEAR_TARGET, D3DCOLOR_RGBA(0, 0, 40, 0xFF), 1.0f, 0);
				m_Device->Clear(0, nullptr, D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
				m_Device->BeginScene();
			}

			YX2_API void EndRender() const
			{
				m_Device->EndScene();
				m_Device->Present(nullptr, nullptr, nullptr, nullptr);
			}

		}; // class Runtime

		class RuntimeReferencee : public NonCopyable
		{
		protected:
			Runtime* const m_Runtime;
			IDirect3DDevice9* const m_Device;

			YX2_API explicit RuntimeReferencee(Runtime* const runtime)
				: m_Runtime(runtime), m_Device(runtime->m_Device)
			{}
		};	// class RuntimeReferencee

		// ***********************************************************************************************
		// Camera management.
		// ***********************************************************************************************

		/** 
		 * \brief A basic static camera.
		 */
		class Camera : public NonCopyable
		{
		protected:
			auto static const g_Width = 1280;
			auto static const g_Height = 720;

		private:
			IDirect3DDevice9* m_Device;
			mat4 m_ViewMatrix;
			mat4 m_ProjectionMatrix;

		public:
			vec3 Eye = vec3(0.0f, 0.0f, -3.0f);
			vec3 Center = vec3(0.0f, 0.0f, 0.0f);
			vec3 Up = vec3(0.0f, 1.0f, 0.0f);

		public:

			/**
			 * \brief Initializes a camera.
			 * \param[in] runtime Engine runtime instance.
			 */
			YX2_API explicit Camera(Runtime const* runtime) : m_Device(runtime->m_Device) { Update(); }

			/** 
			 * \brief Updates camera matrices.
			 */
			YX2_API void Update() const
			{
				m_Device->SetTransform(D3DTS_VIEW, ToD3D(glm::lookAtLH(Eye, Center, Up)));
				m_Device->SetTransform(D3DTS_PROJECTION,
									   ToD3D(glm::perspectiveFovLH<float>(DXM_PI / 4.0f, g_Width, g_Height, 0.01f, 100.0f)));
			}

		}; // class Camera

		/** 
		 * \brief A mouse-control orbital camera with zoom support.
		 */
		class OrbitalCamera final : public Camera
		{
		private:
			float m_CameraRotationYaw = -DXM_PI / 2.0f;
			float m_CameraRotationPitch = 0.0f;
			POINT m_PrevMousePosition = {};

		public:

			/**
			 * \brief Initializes an orbital camera.
			 * \param[in] runtime Engine runtime instance.
			 */
			YX2_API explicit OrbitalCamera(Runtime const* runtime) : Camera(runtime) { Update(); }

			/** 
			 * \brief Updates camera matrices.
			 */
			YX2_API void Update()
			{
				/// @todo Move initial values outside somewhere.
				vec4 cameraRotationCenter(0.0f, 1.24f, 1.5f, 1.0f);
				vec4 cameraCenterOffset(0.0f, 0.0f, -3.0f, 1.0f);
				vec4 cameraUp(0.0f, 1.0f, 0.0f, 1.0f);

				if (GetAsyncKeyState(VK_LBUTTON) != 0)
				{
					// L button is pressed.
					POINT mouseCurrentPosition = {};
					GetCursorPos(&mouseCurrentPosition);

					auto const deltaYaw = static_cast<float>(mouseCurrentPosition.y - m_PrevMousePosition.y) / g_Height;
					auto const deltaPitch = static_cast<float>(mouseCurrentPosition.x - m_PrevMousePosition.x) / g_Width;

					m_CameraRotationYaw += deltaPitch;
					m_CameraRotationPitch = clampf(m_CameraRotationPitch + deltaYaw, -DXM_PI / 12.0f, DXM_PI / 3.0f);
				}
				GetCursorPos(&m_PrevMousePosition);

				auto const cameraTranslation = glm::translate(glm::vec3(cameraRotationCenter));
				auto const cameraRotation = glm::yawPitchRoll(m_CameraRotationYaw, m_CameraRotationPitch, 0.0f);
				cameraCenterOffset = cameraTranslation * cameraRotation * cameraCenterOffset;
				cameraUp = cameraRotation * cameraUp;

				Eye = vec3(cameraCenterOffset);
				Center = vec3(cameraRotationCenter);
				Up = vec3(cameraUp);
				Camera::Update();
			}

		};	// class OrbitalCamera

	} // namespace engine
} // namespace yx2
